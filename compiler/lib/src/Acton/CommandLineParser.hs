{-# LANGUAGE CPP #-}
module Acton.CommandLineParser where

import Options.Applicative

#if defined(darwin_HOST_OS) && defined(aarch64_HOST_ARCH)
defTarget = "aarch64-macos-none"
#elif defined(darwin_HOST_OS) && defined(x86_64_HOST_ARCH)
defTarget = "x86_64-macos-none"
#elif defined(linux_HOST_OS) && defined(aarch64_HOST_ARCH)
defTarget = "aarch64-linux-gnu.2.27"
#elif defined(linux_HOST_OS) && defined(x86_64_HOST_ARCH)
defTarget = "x86_64-linux-gnu.2.27"
#else
#error "Unsupported platform"
#endif

parseCmdLine        :: IO CmdLineOptions
parseCmdLine        = execParser (info (cmdLineParser <**> helper) descr)

data CmdLineOptions = CompileOpt [String] CompileOptions
                    | VersionOpt VersionOptions
                    | CmdOpt Command
                    deriving Show

data VersionOptions = VersionOptions {
                        version :: Bool,
                        numeric_version :: Bool
                     } deriving Show
                     
data Command        = New NewOptions
                    | Build BuildOptions
                    | Cloud CloudOptions
                    | Doc DocOptions
                    deriving Show


data NewOptions     = NewOptions {
                         file      :: String
                    }  deriving Show

data CompileOptions   = CompileOptions {
                         alwaysbuild :: Bool,
                         tty         :: Bool,
                         db          :: Bool,
                         parse       :: Bool,
                         kinds       :: Bool,
                         types       :: Bool,
                         sigs        :: Bool,
                         norm        :: Bool,
                         deact       :: Bool,
                         cps         :: Bool,
                         llift       :: Bool,
                         box         :: Bool,
                         hgen        :: Bool,
                         cgen        :: Bool,
                         ccmd        :: Bool,
                         ty          :: Bool,
                         timing      :: Bool,
                         autostub    :: Bool,
                         stub        :: Bool,
                         cpedantic   :: Bool,
                         quiet       :: Bool,
                         debug       :: Bool,
                         dev         :: Bool,
                         listimports :: Bool,
                         sub         :: Bool,
                         only_build  :: Bool,
                         skip_build  :: Bool,
                         no_threads  :: Bool,
                         root        :: String,
                         tempdir     :: String,
                         syspath     :: String,
                         target      :: String,
                         cpu         :: String,
                         test        :: Bool,
                         searchpath  :: [String]
                     } deriving Show

data BuildOptions = BuildOptions {
                         alwaysB     :: Bool,
                         ttyB        :: Bool,
                         cpedanticB  :: Bool,
                         dbB         :: Bool,
                         no_threadsB :: Bool,
                         debugB      :: Bool,
                         devB        :: Bool,
                         listimportsB :: Bool,
                         only_buildB :: Bool,
                         skip_buildB :: Bool,
                         autostubB   :: Bool,
                         rootB       :: String,
                         ccmdB       :: Bool,
                         quietB      :: Bool,
                         timingB     :: Bool,
                         targetB     :: String,
                         cpuB        :: String,
                         testB       :: Bool,
                         searchpathB :: [String]
                     } deriving Show
                         

data CloudOptions   = CloudOptions {
                         run  :: Bool,
                         list :: Bool,
                         show :: Bool,
                         stop :: Bool
                    }  deriving Show


data DocOptions     = DocOptions {
                         signs :: String,
                         full  :: String
                    }  deriving Show

--------------------------------------------------------------------
-- Internal stuff

cmdLineParser       :: Parser CmdLineOptions
cmdLineParser       = (CmdOpt <$> hsubparser
                        (  command "new"   (info newCommand (progDesc "Create a new Acton project"))
                        <> command "build" (info buildCommand (progDesc "Build an Acton project"))
                        <> command "cloud" (info cloudCommand (progDesc "Run an Acton project in the cloud"))
                        <> command "doc"   (info docCommand (progDesc "Show type and docstring info"))
                      ))
                     <|> (CompileOpt <$> (fmap (:[]) $ argument str (metavar "ACTONFILE" <> help "Compile Acton file" <> completer (bashCompleter "file -X '!*.act' -o plusdirs"))) <*> compileOptions)
                     <|> (VersionOpt <$> versionOptions)

versionOptions         = VersionOptions <$>
                                         switch (long "version"         <> help "Show version information")
                                     <*> switch (long "numeric-version" <> help "Show numeric version")


{-
generalOptions         = GeneralOptions <$> 
                             strOption (long "tempdir" <> metavar "TEMPDIR" <> value "" <> help "Set temporary directory for build files")
                         <*> strOption (long "syspath" <> metavar "TARGETDIR" <>  value "" <> help "Set syspath")
                         <*> switch (long "dev"        <> help "Development mode; include debug symbols etc")
 -}                       

newCommand = New <$> (NewOptions <$> argument (str :: ReadM String) (metavar "PROJECTDIR"))

compileOptions = CompileOptions
        <$> switch (long "always-build" <> help "Show the result of parsing")
        <*> switch (long "tty"          <> help "Act as if run from interactive TTY")
        <*> switch (long "db"           <> help "Enable DB backend")
        <*> switch (long "parse"        <> help "Show the result of parsing")
        <*> switch (long "kinds"        <> help "Show all the result after kind-checking")
        <*> switch (long "types"        <> help "Show all inferred expression types")
        <*> switch (long "sigs"         <> help "Show the inferred type signatures")
        <*> switch (long "norm"         <> help "Show the result after syntactic normalization")
        <*> switch (long "deact"        <> help "Show the result after deactorization")
        <*> switch (long "cps"          <> help "Show the result after CPS conversion")
        <*> switch (long "llift"        <> help "Show the result of lambda-lifting")
        <*> switch (long "box"          <> help "Show the result of (un)boxing")
        <*> switch (long "hgen"         <> help "Show the generated .h header")
        <*> switch (long "cgen"         <> help "Show the generated .c code")
        <*> switch (long "ccmd"         <> help "Show CC / LD commands")
        <*> switch (long "ty"           <> help "Write .ty file to src file directory")
        <*> switch (long "timing"       <> help "Print timing information")
        <*> switch (long "auto-stub"    <> help "Allow automatic stub detection")
        <*> switch (long "stub"         <> help "Stub (.ty) file generation only")
        <*> switch (long "cpedantic"    <> help "Pedantic C compilation with -Werror")
        <*> switch (long "quiet"        <> help "Don't print stuff")
        <*> switch (long "debug"        <> help "Print debug stuff")
        <*> switch (long "dev"          <> help "Development mode; include debug symbols etc")
        <*> switch (long "list-imports" <> help "List module imports")
        <*> switch (long "sub")
        <*> switch (long "only-build"   <> help "Only perform final build of .c files, do not compile .act files")
        <*> switch (long "skip-build"   <> help "Skip final bulid of .c files")
        <*> switch (long "no-threads"   <> help "Don't use threads")
        <*> strOption (long "root"      <> metavar "ROOTACTOR" <> value "" <> help "Set root actor")
        <*> strOption (long "tempdir"   <> metavar "TEMPDIR" <> value "" <> help "Set directory for build files")
        <*> strOption (long "syspath"   <> metavar "TARGETDIR" <> value "" <> help "Set syspath")
        <*> strOption (long "target"    <> metavar "TARGET" <> value defTarget <> help "Target, e.g. x86_64-linux-gnu.2.28")
        <*> strOption (long "cpu"       <> metavar "CPU" <> value "" <> help "CPU, e.g. skylake")
        <*> switch (long "test"         <> help "Build tests")
        <*> many (strOption (long "searchpath" <> metavar "DIR" <> help "Add search path"))

buildCommand          = Build <$> (
    BuildOptions
        <$> switch (long "always-build" <> help "Development mode; include debug symbols etc")
        <*> switch (long "tty"          <> help "Act as if run from interactive TTY")
        <*> switch (long "cpedantic"    <> help "Pedantic C compilation with -Werror")
        <*> switch (long "db"           <> help "Enable DB backend")
        <*> switch (long "no-threads"   <> help "Don't use threads")
        <*> switch (long "debug"        <> help "Print debug stuff")
        <*> switch (long "dev"          <> help "Development mode; include debug symbols etc")
        <*> switch (long "list-imports" <> help "List module imports")
        <*> switch (long "only-build"   <> help "Only perform final build of .c files, do not compile .act files")
        <*> switch (long "skip-build"   <> help "Skip final build of .c files")
        <*> switch (long "auto-stub"    <> help "Allow automatic stub detection")
        <*> strOption (long "root"      <> metavar "ROOTACTOR" <> value "" <> help "Set root actor")
        <*> switch (long "ccmd"         <> help "Show CC / LD commands")
        <*> switch (long "quiet"        <> help "Don't print stuff")
        <*> switch (long "timing"       <> help "Print timing information")
        <*> strOption (long "target"    <> metavar "TARGET" <> value defTarget <> help "Target, e.g. x86_64-linux-gnu.2.28")
        <*> strOption (long "cpu"       <> metavar "CPU" <> value "" <> help "CPU, e.g. skylake")
        <*> switch (long "test"         <> help "Build tests")
        <*> many (strOption (long "searchpath" <> metavar "DIR" <> help "Add search path"))
    )
                 
cloudCommand        = Cloud <$> (
    CloudOptions
        <$> switch (long "run"          <> help "Help run!")
        <*> switch (long "list"         <> help "Help list!")
        <*> switch (long "show"         <> help "Help show!")
        <*> switch (long "stop"         <> help "Help stop!"))

docCommand          = Doc <$> (
    DocOptions
        <$> strOption (long "signs" <> metavar "TYFILE" <> value "" <> help "Show type signatures" <> completer (bashCompleter "file -X '!*.ty' -o plusdirs"))
        <*> strOption (long "full" <> metavar "ACTONFILE" <> value "" <> help "Show type signatures and docstrings" <> completer (bashCompleter "file -X '!*.act' -o plusdirs")))

descr               = fullDesc <> progDesc "Compilation and management of Acton source code and projects"
                      <> header "actonc - the Acton compiler"

-- main = do
--     f <- parseCmdLine
--     print(f)
