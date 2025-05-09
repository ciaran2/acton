{-# LANGUAGE CPP #-}
-- Copyright (C) 2019-2021 Data Ductus AB
--
-- Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
--
-- 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
--
-- 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
--
-- 3. Neither the name of the copyright holder nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
--
-- THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
--

module Main where

import Prelude hiding (readFile, writeFile)

import qualified Acton.Parser
import qualified Acton.Syntax as A
import qualified Acton.CommandLineParser as C
import qualified Acton.Relabel
import qualified Acton.Env
import qualified Acton.QuickType
import qualified Acton.Kinds
import qualified Acton.Types
import Acton.TypeM
import qualified Acton.Solver
import qualified Acton.Normalizer
import qualified Acton.CPS
import qualified Acton.Deactorizer
import qualified Acton.LambdaLifter
import qualified Acton.Boxing
import qualified Acton.CodeGen
import qualified Acton.Builtin
import Utils
import qualified Pretty
import qualified InterfaceFiles

import Control.Applicative
import Control.Concurrent.Async
import Control.Concurrent.MVar
import Control.Exception (throw,catch,displayException,IOException,ErrorCall,try,SomeException)
import Control.Exception (bracketOnError)
import Control.Concurrent (forkIO)
import Control.Monad
import Data.Default.Class (def)
import Data.List.Split
import Data.Monoid ((<>))
import Data.Ord
import Data.Graph
import Data.String.Utils (replace)
import Data.Version (showVersion)
import qualified Data.List
import Error.Diagnose
import Error.Diagnose.Style (defaultStyle)
import qualified Filesystem.Path.CurrentOS as Fsco
import Prettyprinter (unAnnotate)
import Prettyprinter.Render.Text (hPutDoc)
import System.Clock
import System.Directory
import System.Directory.Recursive
import System.Exit
import System.FileLock
import System.FilePath ((</>))
import System.FilePath.Posix
import System.IO hiding (readFile, writeFile)
import System.IO.Temp
import System.Info
import System.Posix.Files
import System.Process
import qualified System.Environment
import qualified System.Exit
import qualified Paths_actonc
import Text.Printf

import qualified Data.ByteString.Char8 as B

main = do
    hSetBuffering stdout LineBuffering
    arg <- C.parseCmdLine
    case arg of
        C.VersionOpt opts       -> printVersion opts
        C.CmdOpt (C.New opts)   -> createProject (C.file opts)
        C.CmdOpt (C.Build opts) -> buildProject $ defaultOpts {
          C.alwaysbuild = C.alwaysB opts,
          C.tty = C.ttyB opts,
          C.autostub = C.autostubB opts,
          C.cpedantic = C.cpedanticB opts,
          C.debug = C.debugB opts,
          C.dev = C.devB opts,
          C.db = C.dbB opts,
          C.listimports = C.listimportsB opts,
          C.only_build = C.only_buildB opts,
          C.skip_build = C.skip_buildB opts,
          C.no_threads = C.no_threadsB opts,
          C.root = C.rootB opts,
          C.ccmd = C.ccmdB opts,
          C.quiet = C.quietB opts,
          C.timing = C.timingB opts,
          C.target = C.targetB opts,
          C.cpu = C.cpuB opts,
          C.test = C.testB opts,
          C.searchpath = C.searchpathB opts
          }
        C.CmdOpt (C.Cloud opts) -> undefined
        C.CmdOpt (C.Doc opts)   -> printDocs opts
        C.CompileOpt nms opts   -> case takeExtension (head nms) of
                                     ".act" -> buildFile opts (head nms)
                                     ".ty" -> printDocs (C.DocOptions (head nms) "")

defaultOpts   = C.CompileOptions False False False False False False False False False False False False
                                 False False False False False False False False False False False False
                                 False False False False "" "" "" C.defTarget "" False []


-- Auxiliary functions ---------------------------------------------------------------------------------------

zig :: Paths -> FilePath
zig paths = sysPath paths ++ "/zig/zig"

cc :: Paths -> C.CompileOptions -> FilePath
cc paths opts = zig paths ++ " cc -target " ++ C.target opts

dump mn h txt      = putStrLn ("\n\n== " ++ h ++ ": " ++ modNameToString mn ++ " ================================\n" ++ txt
                      ++'\n':replicate (38 + length h + length (modNameToString mn)) '=' ++ "\n")

-- Try to acquire a lock, return Nothing if failed, Just (FileLock, FilePath) if succeeded
tryLock :: FilePath -> IO (Maybe (FileLock, FilePath))
tryLock lockPath = do
    maybeLock <- tryLockFile lockPath Exclusive  -- This will fail immediately if locked
    case maybeLock of
        Nothing -> return Nothing  -- Lock failed
        Just lock -> return $ Just (lock, lockPath)

-- The rest of your code can stay the same
-- Try locks sequentially until one succeeds or all fail
findAvailableScratch :: FilePath -> IO (Maybe (FileLock, FilePath))
findAvailableScratch basePath = go [0..31]  -- 32 possible scratch directories
  where
    go [] = return Nothing  -- All attempts failed
    go (x:xs) = do
        let lockPath = joinPath [basePath, "scratch" ++ show x ++ ".lock"]
        result <- tryLock lockPath
        case result of
            Just lockInfo -> return $ Just lockInfo
            Nothing -> go xs  -- Try next number

getModPath :: FilePath -> A.ModName -> FilePath
getModPath path mn =
     joinPath [path, joinPath $ init $ A.modPath mn]


printErrorAndExit msg = do
                  errorWithoutStackTrace msg
                  System.Exit.exitFailure

printErrorAndCleanAndExit msg opts paths = do
                  errorWithoutStackTrace msg
                  cleanup opts paths
                  System.Exit.exitFailure


cleanup opts paths = do
    -- Need platform free path separators
    removeFile (joinPath [projPath paths, ".actonc.lock"])
      `catch` handleNotExists
  where
    handleNotExists :: IOException -> IO ()
    handleNotExists _ = return ()

-- our own readFile & writeFile with hard-coded utf-8 encoding
readFile f = do
    h <- openFile f ReadMode
    hSetEncoding h utf8
    c <- hGetContents h
    return c

writeFile :: FilePath -> String -> IO ()
writeFile f c = do
    h <- openFile f WriteMode
    hSetEncoding h utf8
    hPutStr h c
    hClose h

fmtTime t =
    printf "%6.3f s" secs
  where
    secs :: Float
    secs = (fromIntegral(sec t)) + (fromIntegral (nsec t) / 1000000000)

-- Version handling ------------------------------------------------------------------------------------------

printVersion opts = do
    cv <-  getCcVer
    iff (C.version opts) (putStrLn (showVer cv))
    iff (C.numeric_version opts) (putStrLn getVer)
 
getVer          = showVersion Paths_actonc.version
getVerExtra     = unwords ["compiled by", compilerName, showVersion compilerVersion, "on", os, arch]

getCcVer        = do
    sysPath <- takeDirectory <$> System.Environment.getExecutablePath
    zigPath <- canonicalizePath (sysPath ++ "/../zig/zig")
    verStr <- readProcess zigPath ["version"] []
                `catch` handleNoCc                    -- NOTE: the error is not handled (but actonc would terminate anyhow)
    return $ unwords $ take 1 $ lines verStr
  where handleNoCc :: IOException -> IO String
        handleNoCc e = printErrorAndExit "ERROR: Unable to find cc (the C compiler)\nHINT: Ensure cc is in your PATH"


showVer cv      = "acton " ++ getVer ++ "\n" ++ getVerExtra ++ "\ncc: " ++ cv

printIce errMsg = do ccVer <- getCcVer
                     putStrLn(
                        "ERROR: internal compiler error: " ++ errMsg ++
                        "\nNOTE: this is likely a bug in actonc, please report this at:" ++
                        "\nNOTE: https://github.com/actonlang/acton/issues/new?template=ice.yaml" ++
                        "\nNOTE: acton " ++ getVer ++ " " ++ getVerExtra ++
                        "\nNOTE: cc: " ++ ccVer
                        )

-- Create a project ---------------------------------------------------------------------------------------------

createProject :: String -> IO ()
createProject name = do
    curDir <- getCurrentDirectory
    projDirExists <- doesDirectoryExist name
    iff (projDirExists) $
        printErrorAndExit ("Unable to create project " ++ name ++ ", directory already exists.")
    createDirectoryIfMissing True name
    writeFile (joinPath [ curDir, name, "Acton.toml" ]) ""
    paths <- findPaths (joinPath [ curDir, name, "Acton.toml" ]) defaultOpts
    writeFile (joinPath [ curDir, name, ".gitignore" ]) (
      ".actonc.lock\n" ++
      "build.zig\n" ++
      "build.zig.zon\n" ++
      "out\n"
      )
    writeFile (joinPath [ curDir, name, "README.org" ]) (
      "* " ++ name ++ "\n" ++ name ++ " is a cool Acton project!\n\n\n"
      ++ "** Compile\n\n#+BEGIN_SRC shell\nactonc build\n#+END_SRC\n\n\n"
      ++ "** Run\n\n#+BEGIN_SRC shell\nout/bin/" ++ name ++ "\n#+END_SRC\n\n"
      )
    createDirectoryIfMissing True (srcDir paths)
    writeFile (joinPath [(srcDir paths), name ++ ".act"]) "#\n#\n\nactor main(env):\n    print(\"Hello World!\")\n    env.exit(0)\n"
    putStrLn("Created project " ++ name)
    putStrLn("Enter your new project directory with:\n  cd " ++ name)
    putStrLn("Compile:\n  actonc build")
    putStrLn("Run:\n  ./out/bin/" ++ name)
    gitAvailable <- isGitAvailable
    iff (gitAvailable) $ do
        putStrLn("")
        setCurrentDirectory name
        callProcess "git" ["init"]
        callProcess "git" ["add", "."]
        setCurrentDirectory curDir

-- Build a project -----------------------------------------------------------------------------------------------

buildProject :: C.CompileOptions -> IO ()
buildProject opts = do
                iff (not (null $ (C.root opts)) && (length $ splitOn "." (C.root opts)) == 1) $
                  printErrorAndExit("Project build requires a qualified root actor name, like foo.main")

                -- find all .act files in src/ directory, parse into tasks and
                -- submit for compilation
                curDir <- getCurrentDirectory
                paths <- findPaths (joinPath [ curDir, "Acton.toml" ]) opts
                srcDirExists <- doesDirectoryExist (srcDir paths)
                if not srcDirExists
                  then printErrorAndExit "Missing src/ directory"
                  else do
                    iff (not(quiet opts)) $ do
                      putStrLn("Building project in " ++ projPath paths)
                    -- grab project lock
                    withFileLock (joinPath [projPath paths, ".actonc.lock"]) Exclusive $ \_ -> do
                      allFiles <- getFilesRecursive (srcDir paths)
                      let srcFiles = catMaybes $ map filterActFile allFiles
                      compileFiles opts srcFiles

buildFile :: C.CompileOptions -> FilePath -> IO ()
buildFile opts file = do
    absFile <- canonicalizePath file
    curDir <- getCurrentDirectory
    -- Determine if we are in a project
    projDir <- findProjectDir absFile
    case projDir of
      Just proj -> do
        let relProj = makeRelative curDir proj
        -- In a project, use project directory for compilation
        -- If we are running as a sub-compiler, we just compile directly without
        -- locking since we assume the parent compiler has already locked the
        -- project and may run multiple sub-compilers concurrently
        iff (not(quiet opts)) $ do
          putStrLn("Building file " ++ file ++ " in project " ++ relProj)
        if (C.sub opts)
          then do
            compileFiles opts [file]
          else do
            -- grab project lock
            let lock_file = joinPath [proj, ".actonc.lock"]
            withFileLock lock_file Exclusive $ \_ -> do
              compileFiles opts [file]
      Nothing -> do
        -- Not in a project, use scratch directory for compilation unless
        -- --tempdir is provided - then use that
        if (C.tempdir opts /= "")
          then do
            iff (not(quiet opts)) $ do
              putStrLn("Building file " ++ file ++ " using temporary directory " ++ C.tempdir opts)
            compileFiles opts [file]
          else do
            iff (not(quiet opts)) $ do
              putStrLn("Building file " ++ file ++ " using temporary scratch directory")
            home <- getHomeDirectory
            let basePath = joinPath [home, ".cache", "acton", "scratch"]
            createDirectoryIfMissing True basePath
            maybeLockInfo <- findAvailableScratch basePath
            case maybeLockInfo of
              Nothing -> error "Could not acquire any scratch directory lock"
              Just (lock, lockPath) -> do
                let scratchDir = dropExtension lockPath
                removeDirectoryRecursive scratchDir `catch` handleNotExists
                compileFiles (opts { C.tempdir = scratchDir }) [file]
                unlockFile lock
  where
    handleNotExists :: IOException -> IO ()
    handleNotExists _ = return ()

-- Print documentation -------------------------------------------------------------------------------------------

printDocs :: C.DocOptions -> IO ()
printDocs opts = do
              iff (not (null $ C.signs opts)) $ do
                     let filename = C.signs opts
                         (fileBody,fileExt) = splitExtension $ takeFileName filename
                     case fileExt of
                            ".ty" -> do
                                paths <- findPaths filename defaultOpts
                                env0 <- Acton.Env.initEnv (sysTypes paths) False
                                Acton.Types.showTyFile env0 (modName paths) filename
                            _     -> printErrorAndExit ("Unknown filetype: " ++ filename)
              iff (not (null $ C.full opts)) $
                   putStrLn "Full documentation not implemented"           -- issue #708


-- Compile Acton files ---------------------------------------------------------------------------------------------

findTyFile spaths mn = go spaths
  where
    go []     = return Nothing
    go (p:ps) = do
      let fullPath = joinPath (p : A.modPath mn) ++ ".ty"
      exists <- doesFileExist fullPath
      if exists
        then return (Just fullPath)
        else go ps

removeOrphanFiles :: FilePath -> IO ()
removeOrphanFiles dir = do
    -- Recursively get all files in the "out" directory.
    absOutFiles <- getFilesRecursive dir
    let outFiles = map (makeRelative dir) absOutFiles

    -- Map over each file.
    forM_ outFiles $ \file -> do
        -- Remove the file ending.
        let fileNoExt = dropExtension (dropExtension file)
            srcFile = ("src" </> fileNoExt <.> "act")

        -- If the file is a .root.c file, always remove it and generate a new
        -- one later, if necessary. Only an .act file with a root actor should
        -- have a .root.c file but we cannot judge from here if the .act file
        -- actually has a root actor, so the only safe choice is to remove the
        -- file and let it be recreated if necessary. Should be very cheap
        -- anyway since the file is so small. So check if filename ends with
        -- ".root.c" and remove it!
        if takeExtension file == ".c" && takeExtension (takeBaseName file) == ".root"
          then do
            removeIfExists (dir </> file)
          else do
            -- Check if there is a corresponding .act file in the "src" directory.
            srcExists <- doesFileExist srcFile
            -- If the .act file doesn't exist, remove the file in the "out" directory.
            when (not srcExists) $ removeFile (dir </> file)
              `catch` handleNotExists
  where
        removeIfExists f = removeFile f `catch` handleNotExists
        handleNotExists :: IOException -> IO ()
        handleNotExists _ = return ()

compileFiles :: C.CompileOptions -> [String] -> IO ()
compileFiles opts srcFiles = do
    -- it is ok to get paths from just the first file here since at this point
    -- we only care about project level path stuff and all source files are
    -- known to be in the same project
    paths <- findPaths (head srcFiles) opts

    when (C.debug opts) $ do
        putStrLn ("  Paths:")
        putStrLn ("    sysPath  : " ++ sysPath paths)
        putStrLn ("    sysTypes : " ++ sysTypes paths)
        putStrLn ("    sysLib   : " ++ sysLib paths)
        putStrLn ("    projPath : " ++ projPath paths)
        putStrLn ("    projOut  : " ++ projOut paths)
        putStrLn ("    projTypes: " ++ projTypes paths)
        putStrLn ("    projLib  : " ++ projLib paths)
        putStrLn ("    binDir   : " ++ binDir paths)
        putStrLn ("    srcDir   : " ++ srcDir paths)
        iff (length srcFiles == 1) (putStrLn ("    modName  : " ++ prstr (modName paths)))

    -- remove files in out that do not have corresponding source files!
    removeOrphanFiles (projTypes paths)

    tasks <- mapM (parseActFile opts paths) srcFiles
    iff (C.listimports opts) $ do
        let module_imports = map (\t -> concat [ modNameToString (name t), ": ", (concat $ intersperse " " (map (modNameToString) (importsOf t))) ] ) tasks
        let output = concat $ intersperse "\n" module_imports
        putStrLn output
        System.Exit.exitSuccess

    -- figure out binTasks, if --root is provided, use that, otherwise
    -- presumptuously use all non-stub source compile tasks, which get filtered
    -- out later on (after we parsed the project source files) in case they
    -- don't have a main actor, see filterMainActor
    let rootParts = splitOn "." (C.root opts)
        rootMod   = init rootParts
        guessMod  = if length rootParts == 1 then modName paths else A.modName rootMod
        binTask   = BinTask False (prstr guessMod) (A.GName guessMod (A.name $ last rootParts)) False
        preBinTasks
          | null (C.root opts) = map (\t -> BinTask True (modNameToString (name t)) (A.GName (name t) (A.name "main")) False) (filter (not . stubmode) tasks)
          | otherwise        = [binTask]
        preTestBinTasks = map (\t -> BinTask True (modNameToString (name t)) (A.GName (name t) (A.name "__test_main")) True) (filter (not . stubmode) tasks)
    env <- compileTasks opts paths tasks
    if C.skip_build opts
      then
        putStrLn "  Skipping final build step"
      else
        if C.test opts
          then do
            testBinTasks <- catMaybes <$> mapM (filterMainActor env opts paths) preTestBinTasks
            compileBins opts paths env tasks testBinTasks
            putStrLn "Test executables:"
            mapM_ (\t -> putStrLn (binName t)) testBinTasks
          else do
            compileBins opts paths env tasks preBinTasks
    return ()


-- Paths handling -------------------------------------------------------------------------------------

data Paths      = Paths {
                    searchPath  :: [FilePath],
                    sysPath     :: FilePath,
                    sysTypes    :: FilePath,
                    sysLib      :: FilePath,
                    projPath    :: FilePath,
                    projOut     :: FilePath,
                    projTypes   :: FilePath,
                    projLib     :: FilePath,
                    binDir      :: FilePath,
                    srcDir      :: FilePath,
                    isTmp       :: Bool,
                    fileExt     :: String,
                    modName     :: A.ModName
                  }

-- Given a FILE and optionally --syspath PATH:
-- 'sysPath' is the path to the system directory as given by PATH, defaulting to the actonc executable directory.
-- 'sysTypes' is directory "types" under 'sysPath'.
-- 'sysLib' is directory "lib" under 'sysPath'.
-- 'projPath' is the closest parent directory of FILE that contains an 'Acton.toml' file, or a temporary directory in "/tmp" if no such parent exists.
-- 'projOut' is directory "out" under 'projPath'.
-- 'projTypes' is directory "types" under 'projOut'.
-- 'projLib' is directory "lib" under 'projOut'.
-- 'binDir' is the directory prefix of FILE if 'projPath' is temporary, otherwise it is directory "bin" under 'projOut'
-- 'srcDir' is the directory prefix of FILE if 'projPath' is temporary, otherwise it is directory "src" under 'projPath'
-- 'fileExt' is file suffix of FILE.
-- 'modName' is the module name of FILE (its path after 'src' except 'fileExt', split at every '/')

srcFile                 :: Paths -> A.ModName -> FilePath
srcFile paths mn        = joinPath (srcDir paths : A.modPath mn) ++ ".act"

outBase                 :: Paths -> A.ModName -> FilePath
outBase paths mn        = joinPath (projTypes paths : A.modPath mn)

srcBase                 :: Paths -> A.ModName -> FilePath
srcBase paths mn        = joinPath (srcDir paths : A.modPath mn)

searchPaths :: C.CompileOptions -> [FilePath] -> IO [FilePath]
searchPaths opts deps = do
  -- append /out/types to each dep
  let deps_paths = map (\d -> joinPath [d, "out", "types"]) deps
  return $ deps_paths

findProjectDir :: FilePath -> IO (Maybe FilePath)
findProjectDir path = do
    let configFile = path </> "Acton.toml"
    exists <- doesFileExist configFile
    if exists
        then return (Just path)
        else if path == takeDirectory path  -- Check if we're at root
            then return Nothing
            else findProjectDir (takeDirectory path)


findPaths               :: FilePath -> C.CompileOptions -> IO Paths
findPaths actFile opts  = do execDir <- takeDirectory <$> System.Environment.getExecutablePath
                             sysPath <- canonicalizePath (if null $ C.syspath opts then execDir ++ "/.." else C.syspath opts)
                             let sysLib = joinPath [sysPath, "lib/" ++ if (C.dev opts) then "dev" else "rel"]
                             absSrcFile <- canonicalizePath actFile
                             (isTmp, projPath, dirInSrc) <- analyze (takeDirectory absSrcFile) []
                             let sysTypes = joinPath [sysPath, "base", "out", "types"]
                                 srcDir  = if isTmp then takeDirectory absSrcFile else joinPath [projPath, "src"]
                                 projOut = joinPath [projPath, "out"]
                                 projLib = joinPath [projOut, "lib"]
                                 projTypes = joinPath [projOut, "types"]
                                 binDir  = if isTmp then srcDir else joinPath [projOut, "bin"]
                                 modName = A.modName $ dirInSrc ++ [fileBody]
                             -- join the search paths from command line options with the ones found in the deps directory
                             let sPaths = [projTypes] ++ (C.searchpath opts) ++ [sysTypes]
                             --putStrLn ("Search paths: " ++ show sPaths)
                             createDirectoryIfMissing True binDir
                             createDirectoryIfMissing True projOut
                             createDirectoryIfMissing True projTypes
                             createDirectoryIfMissing True projLib
                             createDirectoryIfMissing True (getModPath projTypes modName)
                             return $ Paths sPaths sysPath sysTypes sysLib projPath projOut projTypes projLib binDir srcDir isTmp fileExt modName
  where (fileBody,fileExt) = splitExtension $ takeFileName actFile

        analyze "/" ds  = do tmp <- canonicalizePath (C.tempdir opts)
                             return (True, tmp, [])
        analyze pre ds  = do exists <- doesFileExist (joinPath [pre, "Acton.toml"])
                             if not exists 
                                then analyze (takeDirectory pre) (takeFileName pre : ds)
                                else case ds of
                                    [] -> return $ (False, pre, [])
                                    "src":dirs -> return $ (False, pre, dirs)
                                    "out":"types":dirs -> return $ (False, pre, dirs)
                                    _ -> error ("************* Source file is not in a valid project directory: " ++ joinPath ds)


-- Handling Acton files -----------------------------------------------------------------------------

filterActFile :: FilePath -> Maybe FilePath
filterActFile file =
    case fileExt of
        ".act" -> Just file
        _ -> Nothing
  where (fileBody, fileExt) = splitExtension $ takeFileName file

parseActFile :: C.CompileOptions -> Paths -> String -> IO CompileTask
parseActFile opts paths actFile = do
                    timeStart <- getTime Monotonic
                    paths <- findPaths actFile opts
                    src <- readFile actFile
                    timeRead <- getTime Monotonic
                    iff (C.timing opts) $ putStrLn("Reading file " ++ makeRelative (srcDir paths) actFile 
                                                   ++ ": " ++ fmtTime(timeRead - timeStart))
                    m <- Acton.Parser.parseModule (modName paths) actFile src
                      `catch` handle "Syntax error" Acton.Parser.parserError "" paths (modName paths)
                      `catch` handle "Context error" Acton.Parser.contextError src paths (modName paths)
                      `catch` handle "Indentation error" Acton.Parser.indentationError src paths (modName paths)
                      `catch` handle "Syntax error" Acton.Parser.failFastError src paths (modName paths)
                    iff (C.parse opts) $ dump (modName paths) "parse" (Pretty.print m)
                    timeParse <- getTime Monotonic
                    iff (C.timing opts) $ putStrLn("Parsing file " ++ makeRelative (srcDir paths) actFile
                                                                   ++ ": " ++ fmtTime(timeParse - timeRead))
                    stubMode <- detectStubMode paths actFile opts
                    return $ ActonTask (modName paths) src m stubMode
    where detectStubMode :: Paths -> String -> C.CompileOptions -> IO Bool
          detectStubMode paths srcfile opts = do
                    exists <- doesFileExist cFile
                    return (exists && C.autostub opts)
              where cFile = replaceExtension srcfile ".c"


-- Compilation tasks, chasing imported modules, compilation and building executables -------------------------------------------

data CompileTask        = ActonTask { name :: A.ModName, src :: String, atree:: A.Module, stubmode :: Bool } deriving (Show)
-- TODO: replace binName String type with ModName just like for CompileTask.
-- ModName is a array so a hierarchy with submodules is represented, we can then
-- get it use joinPath (modPath) to get a path or modName to get a string
-- representation. We need both of BinTask when generating build.zig, so it
-- would be more robust to use that type rather than a hacky character
-- replacement (replaceDot in genBuildZigExe)
data BinTask            = BinTask { isDefaultRoot :: Bool, binName :: String, rootActor :: A.QName, isTest :: Bool } deriving (Show)

-- return task where the specified root actor exists
filterMainActor env opts paths binTask
                         = case Acton.Env.lookupMod m env of
                             Just te -> case lookup n te of
                               Just (A.NAct [] A.TNil{} (A.TRow _ _ _ t A.TNil{}) _)
                                   | prstr t == "Env" || prstr t == "None"
                                      || prstr t == "__builtin__.Env"|| prstr t == "__builtin__.None"-> do   -- TODO: proper check of parameter type
                                      return (Just binTask)
                                   | otherwise -> return Nothing
                               Just t -> return Nothing
                               Nothing -> return Nothing
                             Nothing -> return Nothing
  where mn                  = A.mname qn
        qn@(A.GName m n)    = rootActor binTask
        (sc,_)              = Acton.QuickType.schemaOf env (A.eQVar qn)

importsOf :: CompileTask -> [A.ModName]
importsOf t = A.importsOf (atree t)

compileTasks :: C.CompileOptions -> Paths -> [CompileTask] -> IO Acton.Env.Env0
compileTasks opts paths tasks
                       = do tasks <- chaseImportedFiles opts paths tasks
                            -- We sort out the order of imports etc and split
                            -- out __builtin__, if it's part of the tasks, so we
                            -- can deal with it first
                            let sccs = stronglyConnComp  [(t, name t, importsOf t) | t <- tasks]
                                (builtinSccs, otherSccs) = partition containsBuiltin sccs
                                (as,cs) = Data.List.partition isAcyclic otherSccs

                            -- Seprate compile of __builtin__, if it's part of this project
                            case builtinSccs of
                                [AcyclicSCC t] -> do
                                        builtinEnv0 <- Acton.Env.initEnv (sysTypes paths) True
                                        doTask opts paths builtinEnv0 t
                                        return ()
                                _ -> do return ()
                            let builtinPath = if null builtinSccs then sysTypes paths else projTypes paths

                            -- Compile all the other modules, reinitializing the env from disk
                            if null cs
                             then do env0 <- Acton.Env.initEnv builtinPath False
                                     env1 <- foldM (doTask opts paths) env0 [t | AcyclicSCC t <- as]
                                     return env1
                              else printErrorAndCleanAndExit ("Cyclic imports: "++concatMap showTaskGraph cs) opts paths
  where isAcyclic (AcyclicSCC _) = True
        isAcyclic _              = False
        showTaskGraph ts         = "\n"++concatMap (\t-> concat (intersperse "." (A.modPath (name t)))++" ") ts
        containsBuiltin (AcyclicSCC task) = name task == (A.modName ["__builtin__"])

compileBins:: C.CompileOptions -> Paths -> Acton.Env.Env0 -> [CompileTask] -> [BinTask] -> IO ()
compileBins opts paths env tasks binTasks = do
    iff (not (altOutput opts)) $ do
      zigBuild env opts paths tasks binTasks
    return ()
  where
    handleNotExists :: IOException -> IO ()
    handleNotExists _ = return ()


chaseImportedFiles :: C.CompileOptions -> Paths -> [CompileTask] -> IO [CompileTask]
chaseImportedFiles opts paths itasks
                            = do
                                 let itasks_imps = concatMap importsOf itasks
                                 newtasks <- catMaybes <$> mapM (readAFile itasks) itasks_imps
                                 chaseRecursively (itasks ++ newtasks) (map name newtasks) (concatMap importsOf newtasks)

  where readAFile tasks mn  = case lookUp mn tasks of    -- read and parse file mn in the project directory, unless it is already in tasks 
                                 Just t -> return Nothing
                                 Nothing -> do let actFile = srcFile paths mn
                                               ok <- System.Directory.doesFileExist actFile
                                               if ok then do
                                                   task <- parseActFile opts paths actFile
                                                   return $ Just task
                                                 else return Nothing

        lookUp mn (t : ts)
          | name t == mn     = Just t
          | otherwise        = lookUp mn ts
        lookUp _ []          = Nothing
        
        chaseRecursively tasks mns []
                             = return tasks
        chaseRecursively tasks mns (imn : imns)
                             = if imn `elem` mns
                                then chaseRecursively tasks mns imns
                                else do t <- readAFile tasks imn
                                        chaseRecursively (maybe tasks (:tasks) t)
                                                         (imn:mns)
                                                         (imns ++ concatMap importsOf t)


quiet :: C.CompileOptions -> Bool
quiet opts = C.quiet opts || altOutput opts

doTask :: C.CompileOptions -> Paths -> Acton.Env.Env0 -> CompileTask -> IO Acton.Env.Env0
doTask opts paths env t@(ActonTask mn src m stubMode) = do
    iff (not (quiet opts))  (putStrLn("  Compiling " ++ makeRelative (srcDir paths) actFile
              ++ (if (C.dev opts) then " for development" else " for release")
              ++ (if stubMode then " in stub mode" else "")))

    timeStart <- getTime Monotonic
    -- For stub modules, copy the .c & .h files from src/ to out/types/
    -- Note how this is different from .ext.c style modules
    iff stubMode $ do
        copyFileWithMetadata (replaceExtension actFile ".c") cFile
        copyFileWithMetadata (replaceExtension actFile ".h") hFile

    let outFiles = if stubMode then [tyFile] else [tyFile, hFile, cFile]
    ok <- checkUptoDate opts paths actFile outFiles (importsOf t)
    if C.only_build opts || (ok && not (mn == (modName paths) && (forceCompilation opts)))
      then do
        timeBeforeTy <- getTime Monotonic
        (_,te) <- InterfaceFiles.readFile tyFile
        timeEnd <- getTime Monotonic
        iff (C.timing opts) $ putStrLn("   Read .ty file " ++ makeRelative (projPath paths) tyFile ++ ": " ++ fmtTime(timeEnd - timeBeforeTy))
        iff (not (quiet opts)) $ putStrLn("   Already up to date, in   " ++ fmtTime(timeEnd - timeStart))
        return (Acton.Env.addMod mn te env)
      else do
        createDirectoryIfMissing True (getModPath (projTypes paths) mn)
        env' <- runRestPasses opts paths env m stubMode
          `catch` handle "Compilation error" generalError src paths mn
          `catch` handle "Compilation error" Acton.Env.compilationError src paths mn
          `catch` handleTypeError opts "Type error" Acton.Types.typeError src paths mn
        timeEnd <- getTime Monotonic
        iff (not (quiet opts)) $ putStrLn("   Finished compilation in  " ++ fmtTime(timeEnd - timeStart))
        return env'
  where actFile             = srcFile paths mn
        outbase             = outBase paths mn
        tyFile              = outbase ++ ".ty"
        hFile               = outbase ++ ".h"
        cFile               = outbase ++ ".c"
        forceCompilation :: C.CompileOptions -> Bool
        forceCompilation args = (C.alwaysbuild args) || (C.parse args) || (C.kinds args) || (C.types args) || (C.sigs args)
                                || (C.norm args) || (C.deact args) || (C.cps args) || (C.llift args) || (C.hgen args) ||(C.cgen args)


checkUptoDate :: C.CompileOptions -> Paths -> FilePath -> [FilePath] -> [A.ModName] -> IO Bool
checkUptoDate opts paths actFile outFiles imps = do
    iff (C.debug opts) (putStrLn ("    Checking " ++ makeRelative (srcDir paths) actFile ++ " is up to date..."))
    -- get the path to the actonc binary, i.e. ourself
    actoncBin <- System.Environment.getExecutablePath
    -- get path to `acton` which is the actonc binary without the `c` at the end
    let actonBin = take (length actoncBin - 1) actoncBin
    let potSrcFiles     = [actonBin, actoncBin, actFile, extCFile, srcCFile, srcHFile]
    srcFiles  <- filterM System.Directory.doesFileExist potSrcFiles
    outExists <- mapM System.Directory.doesFileExist outFiles

    if not (and outExists)
        then do
            iff (C.debug opts) (putStrLn ("    Missing output files: " ++ show outExists ++ " for " ++ show outFiles))
            return False
        else do
            -- get the time of the last modified source file
            srcTime  <- head <$> sortBy (comparing Down) <$> mapM System.Directory.getModificationTime srcFiles
            outTiming <- mapM System.Directory.getModificationTime outFiles
            impsOK   <- mapM (impOK (head outTiming)) imps
            return (all (srcTime <) outTiming && and impsOK)
  where
        srcBase         = joinPath [takeDirectory actFile, takeBaseName actFile]
        srcCFile        = srcBase ++ ".c"
        srcHFile        = srcBase ++ ".h"
        extCFile        = srcBase ++ ".ext.c"
        -- except for actFile, these are *potential* source files which might
        -- not actually exist...
        impOK iTime mn  = do
                             impFile <- findTyFile (searchPath paths) mn
                             case impFile of
                               Nothing -> return False
                               Just impFile -> do
                                 impfileTime <- System.Directory.getModificationTime impFile
                                 return (impfileTime < iTime)
        -- find .ty file by looking both in local project and in stdlib
        findTy paths mn = do
                             let localImpName = outBase paths mn ++ ".ty"
                                 stdlibImpName = joinPath (sysTypes paths : A.modPath mn) ++ ".ty"
                             projExist <- System.Directory.doesFileExist localImpName
                             stdlibExist <- System.Directory.doesFileExist stdlibImpName
                             let filePath = case (projExist, stdlibExist) of
                                   (True, True) -> localImpName
                                   (True, False) -> localImpName
                                   (False, True) -> stdlibImpName
                                   (False, False) -> error("ERROR: Unable to find interface file")
                             return filePath

isGitAvailable :: IO Bool
isGitAvailable = do
    (exitCode, _, _) <- readProcessWithExitCode "git" ["--version"] ""
    return $ exitCode == ExitSuccess


-- Check if any other non-standard output is enabled, like --cgen or --sigs
altOutput opts =
  (C.parse opts) || (C.kinds opts) || (C.types opts) || (C.sigs opts) || (C.norm opts) || (C.deact opts) || (C.cps opts) || (C.llift opts) || (C.box opts) || (C.hgen opts) || (C.cgen opts)

runRestPasses :: C.CompileOptions -> Paths -> Acton.Env.Env0 -> A.Module -> Bool -> IO Acton.Env.Env0
runRestPasses opts paths env0 parsed stubMode = do
                      let mn = A.modname parsed
                      let outbase = outBase paths mn
                      let absSrcBase = srcBase paths mn
                      let relSrcBase = makeRelative (projPath paths) (srcBase paths mn)
                      let actFile = absSrcBase ++ ".act"

                      timeStart <- getTime Monotonic

                      envTmp <- Acton.Env.mkEnv (searchPath paths) env0 parsed
                      let env = envTmp { Acton.Env.stub = stubMode }
                      --traceM ("#################### initial env0:")
                      --traceM (Pretty.render (Pretty.pretty env))
                      timeEnv <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Make environment: " ++ fmtTime (timeEnv - timeStart))

                      kchecked <- Acton.Kinds.check env parsed
                      iff (C.kinds opts && mn == (modName paths)) $ dump mn "kinds" (Pretty.print kchecked)
                      timeKindsCheck <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Kinds check     : " ++ fmtTime (timeKindsCheck - timeEnv))

                      (iface,tchecked,typeEnv) <- Acton.Types.reconstruct outbase env kchecked
                      iff (C.types opts && mn == (modName paths)) $ dump mn "types" (Pretty.print tchecked)
                      iff (C.sigs opts && mn == (modName paths)) $ dump mn "sigs" (Acton.Types.prettySigs env mn iface)
                      --traceM ("#################### typed env0:")
                      --traceM (Pretty.render (Pretty.pretty typeEnv))
                      timeTypeCheck <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Type check      : " ++ fmtTime (timeTypeCheck - timeKindsCheck))

                      (normalized, normEnv) <- Acton.Normalizer.normalize typeEnv tchecked
                      iff (C.norm opts && mn == (modName paths)) $ dump mn "norm" (Pretty.print normalized)
                      --traceM ("#################### normalized env0:")
                      --traceM (Pretty.render (Pretty.pretty normEnv))
                      timeNormalized <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Normalizer      : " ++ fmtTime (timeNormalized - timeTypeCheck))

                      (deacted,deactEnv) <- Acton.Deactorizer.deactorize normEnv normalized
                      iff (C.deact opts && mn == (modName paths)) $ dump mn "deact" (Pretty.print deacted)
                      --traceM ("#################### deacted env0:")
                      --traceM (Pretty.render (Pretty.pretty deactEnv))
                      timeDeactorizer <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Deactorizer     : " ++ fmtTime (timeDeactorizer - timeNormalized))

                      (cpstyled,cpsEnv) <- Acton.CPS.convert deactEnv deacted
                      iff (C.cps opts && mn == (modName paths)) $ dump mn "cps" (Pretty.print cpstyled)
                      --traceM ("#################### cps'ed env0:")
                      --traceM (Pretty.render (Pretty.pretty cpsEnv))
                      timeCPS <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: CPS             : " ++ fmtTime (timeCPS - timeDeactorizer))

                      (lifted,liftEnv) <- Acton.LambdaLifter.liftModule cpsEnv cpstyled
                      iff (C.llift opts && mn == (modName paths)) $ dump mn "llift" (Pretty.print lifted)
                      --traceM ("#################### lifteded env0:")
                      --traceM (Pretty.render (Pretty.pretty liftEnv))
                      timeLLift <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Lambda Lifting  : " ++ fmtTime (timeLLift - timeCPS))

                      boxed <- Acton.Boxing.doBoxing liftEnv lifted
                      iff (C.box opts && mn == (modName paths)) $ dump mn "box" (Pretty.print boxed)
                      timeBoxing <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Boxing :          " ++ fmtTime (timeBoxing - timeLLift))

                      (n,h,c) <- Acton.CodeGen.generate liftEnv relSrcBase boxed
                      timeCodeGen <- getTime Monotonic
                      iff (C.timing opts) $ putStrLn("    Pass: Generating code : " ++ fmtTime (timeCodeGen - timeBoxing))

                      iff (C.hgen opts) $ do
                          putStrLn(h)
                          System.Exit.exitSuccess
                      iff (C.cgen opts) $ do
                          putStrLn(c)
                          System.Exit.exitSuccess

                      iff (altOutput opts || not stubMode) (do
                          let cFile = outbase ++ ".c"
                              hFile = outbase ++ ".h"

                          writeFile hFile h
                          writeFile cFile c
                          let tyFileName = modNameToString(modName paths) ++ ".ty"
                          iff (C.ty opts) $
                               copyFileWithMetadata (joinPath [projTypes paths, tyFileName]) (joinPath [srcDir paths, tyFileName])

                          timeCodeWrite <- getTime Monotonic
                          iff (C.timing opts) $ putStrLn("    Pass: Writing code    : " ++ fmtTime (timeCodeWrite - timeCodeGen))
                                                           )

                      return $ Acton.Env.addMod mn iface (env0 `Acton.Env.withModulesFrom` env)

handle errKind f src paths mn ex = do
    putStrLn ("\nERROR: Error when compiling " ++ (prstr mn) ++ " module: " ++ errKind)
    putStrLn (Acton.Parser.makeReport (f ex) src)
    handleCleanup paths mn

modNameToFilename :: A.ModName -> String
modNameToFilename mn = joinPath (map nameToString names) ++ ".act"
  where
    A.ModName names = mn

handleTypeError opts errKind f src paths mn ex = do
    printDiag opts $ mkErrorDiagnostic (modNameToFilename mn) src (typeReport ex (modNameToFilename mn) src)
    handleCleanup paths mn

handleCleanup paths mn = do
    removeIfExists (outbase++".ty")
    System.Exit.exitFailure
  where outbase        = outBase paths mn
        removeIfExists f = removeFile f `catch` handleNotExists
        handleNotExists :: IOException -> IO ()
        handleNotExists _ = return ()

printDiag :: C.CompileOptions -> Diagnostic String -> IO ()
printDiag opts d = do
    -- TODO: change to print to stderr! current tests presume stdout so we print to stdout for now..
    tty <- hIsTerminalDevice stdout
    if tty || (C.tty opts)
      then printDiagnostic stdout WithUnicode (TabSize 4) defaultStyle d
      else hPutDoc stdout $ unAnnotate (prettyDiagnostic WithoutUnicode (TabSize 4) d)

writeRootC :: Acton.Env.Env0 -> C.CompileOptions -> Paths -> BinTask -> IO (Maybe BinTask)
writeRootC env opts paths binTask = do
    let qn@(A.GName m n) = rootActor binTask
        mn = A.mname qn
        outbase = outBase paths mn
        rootFile = if (isTest binTask) then outbase ++ ".test_root.c" else outbase ++ ".root.c"
    case Acton.Env.lookupMod m env of
        Nothing -> return Nothing  -- Handle the case where module lookup fails
        Just modEnv ->
            case lookup n modEnv of
                Just (A.NAct [] A.TNil{} (A.TRow _ _ _ t A.TNil{}) _)
                    | prstr t == "Env" || prstr t == "None"
                        || prstr t == "__builtin__.Env"|| prstr t == "__builtin__.None" -> do
                        c <- Acton.CodeGen.genRoot env qn
                        createDirectoryIfMissing True (takeDirectory rootFile)
                        writeFile rootFile c
                        return (Just binTask)
                    | otherwise -> handle "Type error" Acton.Types.typeError "" paths m
                        (Acton.Types.TypeError NoLoc ("Illegal type "++ prstr t ++ " of parameter to root actor " ++ prstr qn))
                Just t -> handleTypeError opts "Type error" Acton.Types.typeError "" paths m
                    (Acton.Types.TypeError NoLoc (prstr qn ++ " has not actor type."))
                Nothing -> return Nothing

modNameToString :: A.ModName -> String
modNameToString (A.ModName names) = intercalate "." (map nameToString names)

nameToString :: A.Name -> String
nameToString (A.Name _ s) = s

isWindowsOS :: String -> Bool
isWindowsOS targetTriple = case splitOn "-" targetTriple of
    (_:os:_) -> os == "windows"
    _        -> False

runZig opts zigCmd paths wd = do
    iff (C.ccmd opts) $ putStrLn zigCmd
    (returnCode, zigStdout, zigStderr) <- readCreateProcessWithExitCode (shell $ zigCmd){ cwd = wd } ""
    case returnCode of
        ExitSuccess -> do
          iff (C.debug opts) $ putStrLn zigStderr
          return ()
        ExitFailure ret -> do
          printIce ("compilation of generated Zig code failed, returned error code" ++ show ret)
          putStrLn $ "zig stdout:\n" ++ zigStdout
          putStrLn $ "zig stderr:\n" ++ zigStderr
          cleanup opts paths
          System.Exit.exitFailure

makeAlwaysRelative :: FilePath -> FilePath -> FilePath
makeAlwaysRelative base target =
    case makeRelative base target of
        path | isAbsolute path ->  -- Still an absolute path, so no overlap found
               let baseCount = length $ filter (/= "./") $ splitPath base
                   targetPath = dropDrive path  -- Remove the drive part of the absolute path
               in joinPath (replicate baseCount "..") </> targetPath
            | otherwise -> path  -- makeRelative found overlap, use its result

-- TODO: replace all of this with generic+crypto?!
#if defined(darwin_HOST_OS) && defined(aarch64_HOST_ARCH)
defCpu = " -Dcpu=apple_a15 "
#elif defined(darwin_HOST_OS) && defined(x86_64_HOST_ARCH)
defCpu = ""
#elif defined(linux_HOST_OS) && defined(aarch64_HOST_ARCH)
defCpu = " -Dcpu=cortex_a72 "
#elif defined(linux_HOST_OS) && defined(x86_64_HOST_ARCH)
defCpu = ""
#else
#error "Unsupported platform"
#endif

zigBuild :: Acton.Env.Env0 -> C.CompileOptions -> Paths -> [CompileTask] -> [BinTask] -> IO ()
zigBuild env opts paths tasks binTasks = do
    allBinTasks <- mapM (writeRootC env opts paths) binTasks
    let realBinTasks = catMaybes allBinTasks
    iff (not (quiet opts)) $ putStrLn("  Final compilation step")
    timeStart <- getTime Monotonic

    -- custom build.zig ?
    homeDir <- getHomeDirectory
    let local_cache_dir = joinPath [ homeDir, ".cache", "acton", "zig-local-cache" ]
        global_cache_dir = joinPath [ homeDir, ".cache", "acton", "zig-global-cache" ]
        no_threads = if isWindowsOS (C.target opts) then True else C.no_threads opts
        target_cpu = if (C.cpu opts /= "")
                       then " -Dcpu=" ++ C.cpu opts
                       else
                         case (splitOn "-" (C.target opts)) of
                           ("native":_)            -> defCpu
                           ("aarch64":"macos":_)   -> " -Dcpu=apple_a15 "
    -- TODO: how do we do better here? Windows presumably runs on many CPUs that are not aarch64. We really just want to enable AES
                           ("aarch64":"windows":_) -> " -Dcpu=apple_a15 "
                           ("aarch64":"linux":_)   -> " -Dcpu=cortex_a72 "
                           ("x86_64":_:_)          -> " -Dcpu=westmere "
                           (_:_:_)                 -> defCpu
        buildZigPath = joinPath [projPath paths, "build.zig"]
        buildZonPath = joinPath [projPath paths, "build.zig.zon"]

    buildZigExists <- doesFileExist buildZigPath
    buildZonExists <- doesFileExist buildZonPath
    -- Compute relative path from current directory (projPath paths)
    let relativeSysPath = makeAlwaysRelative (projPath paths) (sysPath paths)
    iff (not buildZigExists) $ do
      let distBuildZigPath = joinPath [(sysPath paths), "builder", "build.zig"]
      copyFile distBuildZigPath buildZigPath
    if buildZonExists
      then do
        curBuildZon <- readFile buildZonPath
        let newBuildZon = replace "SYSPATH" relativeSysPath curBuildZon
        removeFile buildZonPath
          `catch` handleNotExists
        writeFile buildZonPath newBuildZon
      else do
        let distBuildZonPath = joinPath [(sysPath paths), "builder", "build.zig.zon"]
        distBuildZon <- readFile distBuildZonPath
        let buildZon = replace "SYSPATH" relativeSysPath distBuildZon
        writeFile buildZonPath buildZon

    let zigCmdBase = zig paths ++ " build " ++ " --cache-dir " ++ local_cache_dir ++
                 " --global-cache-dir " ++ global_cache_dir ++
                 (if (C.debug opts) then " --verbose " else "")
    let zigCmd = zigCmdBase ++
                 " --prefix " ++ projOut paths ++ " --prefix-exe-dir 'bin'" ++
                 (if (C.debug opts) then " --verbose " else "") ++
                 " -Dtarget=" ++ (C.target opts) ++
                 target_cpu ++
                 " -Doptimize=" ++ (if (C.dev opts) then "Debug" else "ReleaseFast") ++
                 (if (C.db opts) then " -Ddb " else "") ++
                 (if no_threads then " -Dno_threads " else "") ++
                 (if (C.cpedantic opts) then " -Dcpedantic " else "")

    iff (C.debug opts) $ putStrLn ("zigCmd: " ++ zigCmd)
    runZig opts zigCmd paths (Just (projPath paths))
    -- if we are in a temp acton project, copy the outputted binary next to the source file
    if (isTmp paths && not (null realBinTasks))
      then do
        let baseName   = binName (head binTasks)
            exeName    = if isWindowsOS (C.target opts) then baseName ++ ".exe" else baseName
            srcBinFile = joinPath [ projOut paths, "bin", exeName ]
            dstBinFile = joinPath [ binDir paths, exeName ]
        copyFile srcBinFile dstBinFile
      else return ()
    cleanup opts paths
    timeEnd <- getTime Monotonic
    iff (not (quiet opts)) $ putStrLn("   Finished final compilation step in  " ++ fmtTime(timeEnd - timeStart))
    return ()
  where
    handleNotExists :: IOException -> IO ()
    handleNotExists _ = return ()
