module Acton.Builder where

buildzig :: String
buildzig = "// Acton Project Builder\n// Performs the final build of the project by compiling the generated C code.\n\nconst std = @import(\"std\");\nconst print = @import(\"std\").debug.print;\nconst ArrayList = std.ArrayList;\n\npub const FilePath = struct {\n    filename: []const u8,\n    full_path: []const u8,\n    dir: []const u8,\n    file_path: []const u8,\n    test_root: bool\n};\n\n// We have an absolute path we want to get to, but we have to provide it as a\n// relative path from the current position. The easiest way to do this is to go\n// up the directory tree until we're at the root, and then the absolute path is\n// relative to the root and can be used. It would be more elegant to figure out\n// if there are actual commonalities between the paths and only traverse\n// upwards as far as necessary.\nfn relJoinPath(allocator: std.mem.Allocator, dots: []const u8, base: []const u8, relative: []const u8) []const u8 {\n    const path = allocator.alloc(u8, dots.len + base.len + relative.len + 1) catch @panic(\"OOM\");\n    _ = std.fmt.bufPrint(path, \"{s}{s}/{s}\", .{dots, base, relative}) catch @panic(\"Error joining paths\");\n    return path;\n}\n\nfn joinPath(allocator: std.mem.Allocator, base: []const u8, relative: []const u8) []const u8 {\n    const path = allocator.alloc(u8, base.len + relative.len + 1) catch @panic(\"OOM\");\n    _ = std.fmt.bufPrint(path, \"{s}/{s}\", .{base, relative}) catch @panic(\"Error joining paths\");\n    return path;\n}\n\nfn dotsToRoot(allocator: std.mem.Allocator, cwd: []const u8) []const u8 {\n    // Split up the path into its components, separated by std.fs.path.sep\n    var parts = std.mem.splitScalar(u8, cwd, std.fs.path.sep);\n    var num_parts: u16 = 0;\n    while (parts.next()) |_| {\n        num_parts += 1;\n    }\n    num_parts -= 1;\n    var dotpath = allocator.alloc(u8, 3*num_parts) catch @panic(\"OOM\");\n    var i: u16 = 0;\n    while (i < num_parts) : (i += 1) {\n        dotpath[i*3+0] = '.';\n        dotpath[i*3+1] = '.';\n        dotpath[i*3+2] = std.fs.path.sep;\n    }\n    return dotpath;\n}\n\npub fn build(b: *std.Build) void {\n    const buildroot_path = b.build_root.handle.realpathAlloc(b.allocator, \".\") catch @panic(\"ASD\");\n    const dots_to_root = dotsToRoot(b.allocator, buildroot_path);\n    defer b.allocator.free(dots_to_root);\n    const optimize = b.standardOptimizeOption(.{});\n    const target = b.standardTargetOptions(.{});\n    const db = b.option(bool, \"db\", \"\") orelse false;\n    const only_lib = b.option(bool, \"only_lib\", \"\") orelse false;\n    const no_threads = b.option(bool, \"no_threads\", \"\") orelse false;\n    const syspath = b.option([]const u8, \"syspath\", \"\") orelse \"\";\n    const arg_deps_path = b.option([]const u8, \"deps_path\", \"\") orelse \"\";\n\n    const deps_path = if (arg_deps_path.len > 0) arg_deps_path else joinPath(b.allocator, buildroot_path, \"deps\");\n\n    const projpath_outtypes = joinPath(b.allocator, buildroot_path, \"out/types\");\n    const syspath_base = relJoinPath(b.allocator, dots_to_root, syspath, \"base\");\n    const syspath_include = joinPath(b.allocator, syspath, \"depsout/include\");\n    const syspath_lib = joinPath(b.allocator, syspath, \"depsout/lib\");\n\n    print(\"Acton Project Builder - building {s}\\nDeps path: {s}\\n\", .{buildroot_path, deps_path});\n\n    var iter_dir = b.build_root.handle.openDir(\n        \"out/types/\", .{ .iterate = true },\n    ) catch |err| {\n        std.log.err(\"Error opening iterable dir: {}\", .{err});\n        std.os.exit(1);\n    };\n\n    var c_files = ArrayList([]const u8).init(b.allocator);\n    var root_c_files = ArrayList(*FilePath).init(b.allocator);\n    var walker = iter_dir.walk(b.allocator) catch |err| {\n        std.log.err(\"Error walking dir: {}\", .{err});\n        std.os.exit(1);\n    };\n    defer walker.deinit();\n\n    // Find all .c files\n    while (true) {\n        const next_result = walker.next() catch |err| {\n            std.log.err(\"Error getting next: {}\", .{err});\n            std.os.exit(1);\n        };\n        if (next_result) |entry| {\n            if (entry.kind == .file) {\n                if (std.mem.endsWith(u8, entry.basename, \".c\")) {\n                    const fPath = b.allocator.create(FilePath) catch |err| {\n                        std.log.err(\"Error allocating FilePath entry: {}\", .{err});\n                        std.os.exit(1);\n                    };\n                    const full_path = entry.dir.realpathAlloc(b.allocator, entry.basename) catch |err| {\n                        std.log.err(\"Error getting dir name: {}\", .{err});\n                        std.os.exit(1);\n                    };\n                    const dir = entry.dir.realpathAlloc(b.allocator, \".\") catch |err| {\n                        std.log.err(\"Error getting dir name: {}\", .{err});\n                        std.os.exit(1);\n                    };\n                    fPath.full_path = full_path;\n                    fPath.dir = dir;\n                    fPath.filename = b.allocator.dupe(u8, entry.basename) catch |err| {\n                        std.log.err(\"Error allocating filename entry: {}\", .{err});\n                        std.os.exit(1);\n                    };\n                    const file_path = b.allocator.alloc(u8, full_path.len - projpath_outtypes.len) catch |err| {\n                        std.log.err(\"Error allocating file_path entry: {}\", .{err});\n                        std.os.exit(1);\n                    };\n                    @memcpy(file_path, full_path[projpath_outtypes.len..]);\n                    fPath.file_path = file_path;\n\n                    print(\"-- filename : {s}\\n\", .{fPath.filename});\n                    print(\"   full_path: {s}\\n\", .{fPath.full_path});\n                    print(\"   dir      : {s}\\n\", .{fPath.dir});\n                    print(\"   file_path: {s}\\n\", .{fPath.file_path});\n\n                    if (std.mem.endsWith(u8, entry.basename, \".root.c\")) {\n                        fPath.test_root = false;\n                        root_c_files.append(fPath) catch |err| {\n                            std.log.err(\"Error appending to root .c files: {}\", .{err});\n                            std.os.exit(1);\n                        };\n                    } else if (std.mem.endsWith(u8, entry.basename, \".test_root.c\")) {\n                        fPath.test_root = true;\n                        root_c_files.append(fPath) catch |err| {\n                            std.log.err(\"Error appending to test_root .c files: {}\", .{err});\n                            std.os.exit(1);\n                        };\n                    } else {\n                        c_files.append(fPath.full_path) catch |err| {\n                            std.log.err(\"Error appending to .c files: {}\", .{err});\n                            std.os.exit(1);\n                        };\n                    }\n                }\n            }\n        } else {\n            break;\n        }\n    }\n\n    const libActonProject = b.addStaticLibrary(.{\n        .name = \"ActonProject\",\n        .target = target,\n        .optimize = optimize,\n    });\n    var flags = std.ArrayList([]const u8).init(b.allocator);\n    defer flags.deinit();\n\n    var file_prefix_map = std.ArrayList(u8).init(b.allocator);\n    defer file_prefix_map.deinit();\n    const file_prefix_path = b.build_root.handle.openDir(\"..\", .{}) catch unreachable;\n    const file_prefix_path_path = file_prefix_path.realpathAlloc(b.allocator, \".\") catch unreachable;\n    file_prefix_map.appendSlice(\"-ffile-prefix-map=\") catch unreachable;\n    file_prefix_map.appendSlice(file_prefix_path_path) catch unreachable;\n    file_prefix_map.appendSlice(\"/=\") catch unreachable;\n    flags.append(file_prefix_map.items) catch unreachable;\n\n    if (optimize == .Debug) {\n        print(\"Debug build\\n\", .{});\n        flags.appendSlice(&.{\n            \"-DDEV\",\n        }) catch |err| {\n            std.log.err(\"Error appending flags: {}\", .{err});\n            std.os.exit(1);\n        };\n    }\n\n    if (db)\n        flags.appendSlice(&.{\"-DACTON_DB\",}) catch unreachable;\n\n    if (no_threads) {\n        print(\"No threads\\n\", .{});\n    } else {\n        print(\"Threads enabled\\n\", .{});\n        flags.appendSlice(&.{\n            \"-DACTON_THREADS\",\n        }) catch |err| {\n            std.log.err(\"Error appending flags: {}\", .{err});\n            std.os.exit(1);\n        };\n    }\n\n    for (c_files.items) |entry| {\n        libActonProject.addCSourceFile(.{ .file = .{ .path = entry }, .flags = flags.items });\n    }\n\n    libActonProject.addIncludePath(.{ .path = buildroot_path });\n\n    // project dependencies\n    print(\"Checking for dependencies in: {s}\\n\", .{deps_path});\n    const deps_dir = std.fs.cwd().openDir(deps_path, .{ .iterate = true });\n    if (deps_dir) |dir| {\n        //defer dir.close();\n        var deps_walker = dir.iterate();\n        while (deps_walker.next() catch unreachable) |dep_entry| {\n\n            if (dep_entry.kind == .directory) {\n                std.debug.print(\"Found sub-directory: {s}\\n\", .{dep_entry.name});\n                const dep_path = joinPath(b.allocator, deps_path, dep_entry.name);\n                libActonProject.addIncludePath(.{ .path = dep_path });\n            }\n        }\n    } else |err| {\n        std.debug.print(\"Failed to open directory: {}\\n\", .{err});\n    }\n\n    libActonProject.addIncludePath(.{ .path = syspath_base });\n    libActonProject.addIncludePath(.{ .path = syspath_include });\n    libActonProject.linkLibC();\n    libActonProject.linkLibCpp();\n    b.installArtifact(libActonProject);\n\n    if (!only_lib) {\n        const libactondb_dep = b.dependency(\"actondb\", .{\n            .target = target,\n            .optimize = optimize,\n            .syspath_include = syspath_include,\n        });\n\n        const actonbase_dep = b.dependency(\"base\", .{\n            .target = target,\n            .optimize = optimize,\n            .no_threads = no_threads,\n            .db = db,\n            .syspath = syspath,\n        });\n\n        const dep_libgc = b.dependency(\"libgc\", .{\n            .target = target,\n            .optimize = optimize,\n            .BUILD_SHARED_LIBS = false,\n            .enable_large_config = true,\n            .enable_mmap = true,\n        });\n\n        // -- ActonDeps ------------------------------------------------------------\n        const dep_libargp = b.dependency(\"libargp\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libbsdnt = b.dependency(\"libbsdnt\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libmbedtls = b.dependency(\"libmbedtls\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libnetstring = b.dependency(\"libnetstring\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libpcre2 = b.dependency(\"libpcre2\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libprotobuf_c = b.dependency(\"libprotobuf_c\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libtlsuv = b.dependency(\"libtlsuv\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libutf8proc = b.dependency(\"libutf8proc\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libuuid = b.dependency(\"libuuid\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libuv = b.dependency(\"libuv\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libxml2 = b.dependency(\"libxml2\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libyyjson = b.dependency(\"libyyjson\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        const dep_libsnappy_c = b.dependency(\"libsnappy\", .{\n            .target = target,\n            .optimize = optimize,\n        });\n\n        for (root_c_files.items) |entry| {\n            // Get the binary name, by removing .root.c from end and having it relative to the projpath_outtypes\n            var nlen = \".root.c\".len;\n            if (entry.test_root)\n                nlen = \".test_root.c\".len - \".test_\".len;\n            nlen += 1; // for the null terminator\n            const binname = b.allocator.alloc(u8, entry.file_path.len-nlen) catch |err| {\n                std.log.info(\"Error allocating binname: {}\", .{err});\n                std.os.exit(1);\n            };\n            if (entry.test_root) {\n                // Write '.test_' to start of binname\n                const buf = std.fmt.allocPrint(b.allocator, \".test_{s}\", .{entry.file_path[1..entry.file_path.len - \".test_root.c\".len]}) catch |err| {\n                    std.log.err(\"Error allocating binname: {}\", .{err});\n                    std.os.exit(1);\n                };\n                @memcpy(binname, buf);\n            } else {\n                @memcpy(binname, entry.file_path[1..(entry.file_path.len - \".root.c\".len)]);\n            }\n            // Replace / with . in the binary name\n            for (binname) |*ch| {\n                if (ch.* == '/') ch.* = '.';\n            }\n\n            print(\"Building executable from: {s} -> {s}\\n\", .{entry.full_path, binname});\n\n            const executable = b.addExecutable(.{\n                .name = binname,\n                .target = target,\n                .optimize = optimize,\n            });\n            //_ = syspath;\n            executable.addCSourceFile(.{ .file = .{ .path = entry.full_path }, .flags = flags.items });\n            executable.addIncludePath(.{ .path = buildroot_path });\n            executable.addIncludePath(.{ .path = syspath_base });\n            executable.addIncludePath(.{ .path = syspath_include });\n            executable.addLibraryPath(.{ .path = syspath_lib });\n            executable.linkLibrary(libActonProject);\n\n            // project dependencies\n            if (deps_dir) |dir| {\n                //defer dir.close();\n                var deps_walker = dir.iterate();\n                while (deps_walker.next() catch unreachable) |dep_entry| {\n\n                    if (dep_entry.kind == .directory) {\n                        std.debug.print(\"Found sub-directory: {s}\\n\", .{dep_entry.name});\n                        const dep_path = joinPath(b.allocator, deps_path, dep_entry.name);\n                        executable.addIncludePath(.{ .path = dep_path });\n                        const dep_path_rel = joinPath(b.allocator, \"deps\", dep_entry.name);\n                        const dep_dep = b.dependency(dep_path_rel, .{\n                            .target = target,\n                            .optimize = optimize,\n                            .only_lib = true,\n                            .syspath = syspath,\n                            .deps_path = deps_path,\n                        });\n                        executable.linkLibrary(dep_dep.artifact(\"ActonProject\"));\n                    }\n                }\n            } else |err| {\n                std.debug.print(\"Failed to open directory: {}\\n\", .{err});\n            }\n\n            executable.linkLibrary(actonbase_dep.artifact(\"Acton\"));\n            if (db) {\n                executable.linkLibrary(libactondb_dep.artifact(\"ActonDB\"));\n                executable.linkLibrary(dep_libargp.artifact(\"argp\"));\n                executable.linkLibrary(dep_libuuid.artifact(\"uuid\"));\n            }\n            executable.linkLibrary(dep_libbsdnt.artifact(\"bsdnt\"));\n            executable.linkLibrary(dep_libmbedtls.artifact(\"mbedcrypto\"));\n            executable.linkLibrary(dep_libmbedtls.artifact(\"mbedtls\"));\n            executable.linkLibrary(dep_libmbedtls.artifact(\"mbedx509\"));\n            executable.linkLibrary(dep_libnetstring.artifact(\"netstring\"));\n            executable.linkLibrary(dep_libpcre2.artifact(\"pcre2\"));\n            executable.linkLibrary(dep_libprotobuf_c.artifact(\"protobuf-c\"));\n            executable.linkLibrary(dep_libsnappy_c.artifact(\"snappy-c\"));\n            executable.linkLibrary(dep_libtlsuv.artifact(\"tlsuv\"));\n            executable.linkLibrary(dep_libutf8proc.artifact(\"utf8proc\"));\n            executable.linkLibrary(dep_libuv.artifact(\"uv\"));\n            executable.linkLibrary(dep_libxml2.artifact(\"xml2\"));\n            executable.linkLibrary(dep_libyyjson.artifact(\"yyjson\"));\n            executable.linkLibrary(dep_libgc.artifact(\"gc\"));\n\n            executable.linkLibC();\n            executable.linkLibCpp();\n            b.installArtifact(executable);\n        }\n    }\n}\n"

buildzigzon :: String
buildzigzon = ".{\n    .name = \"actonproject\",\n    .version = \"0.0.0\",\n    .dependencies = .{\n        .actondb = .{\n            .path = \".build/sys/backend/\",\n        },\n        .base = .{\n            .path = \".build/sys/base/\",\n        },\n        .libargp = .{\n            .path = \".build/sys/deps/libargp/\",\n        },\n        .libbsdnt = .{\n            .path = \".build/sys/deps/libbsdnt/\",\n        },\n        .libgc = .{\n            .path = \".build/sys/deps/libgc/\",\n        },\n        .libmbedtls = .{\n            .path = \".build/sys/deps/mbedtls/\",\n        },\n        .libnetstring = .{\n            .path = \".build/sys/deps/libnetstring/\",\n        },\n        .libpcre2 = .{\n            .path = \".build/sys/deps/pcre2/\",\n        },\n        .libprotobuf_c = .{\n            .path = \".build/sys/deps/libprotobuf_c/\",\n        },\n        .libsnappy = .{\n            .path = \".build/sys/deps/libsnappy_c/\",\n        },\n        .libtlsuv = .{\n            .path = \".build/sys/deps/tlsuv/\",\n        },\n        .libutf8proc = .{\n            .path = \".build/sys/deps/libutf8proc/\",\n        },\n        .libuuid = .{\n            .path = \".build/sys/deps/libuuid/\",\n        },\n        .libuv = .{\n            .path = \".build/sys/deps/libuv/\",\n        },\n        .libxml2 = .{\n            .path = \".build/sys/deps/libxml2/\",\n        },\n        .libyyjson = .{\n            .path = \".build/sys/deps/libyyjson/\",\n        },\n    },\n    .paths = .{\"\"},\n}\n"
