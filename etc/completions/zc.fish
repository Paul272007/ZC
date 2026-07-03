# zc fish completion

function __zc_needs_command
    set cmd (commandline -opc)
    if test (count $cmd) -eq 1
        return 0
    end
    return 1
end

function __zc_using_command
    set cmd (commandline -opc)
    if test (count $cmd) -gt 1
        if test $cmd[2] = $argv[1]
            return 0
        end
    end
    return 1
end

function __zc_using_subcommand
    set cmd (commandline -opc)
    if test (count $cmd) -gt 2
        if test $cmd[2] = $argv[1]; and test $cmd[3] = $argv[2]
            return 0
        end
    end
    return 1
end

# Global Options
complete -c zc -n '__zc_needs_command' -s v -l version -d 'Show version'
complete -c zc -n '__zc_needs_command' -s h -l help -d 'Show help'

# Commands
set -l commands run create init setup build add remove use publish clean list install uninstall update login logout config languages
for c in $commands
    complete -c zc -n '__zc_needs_command' -a $c
end

# All subcommands have -h / --help
complete -c zc -n 'not __zc_needs_command' -s h -l help -d 'Show help'

# run
complete -c zc -n '__zc_using_command run' -s a -l args -d 'Arguments passed to the program'
complete -c zc -n '__zc_using_command run' -s f -l force -d 'Force compiling even if target already exists'
complete -c zc -n '__zc_using_command run' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command run' -s k -l keep -d 'Do not delete executable after being executed'
complete -c zc -n '__zc_using_command run' -s p -l plus -d 'Force compilation as C++'
complete -c zc -n '__zc_using_command run' -s s -l static -d 'Compile prioritizing the use of static libraries'
complete -c zc -n '__zc_using_command run' -s E -l preprocess -d 'Preprocess only'
complete -c zc -n '__zc_using_command run' -s S -l assemble -d 'Compile, but do not assemble or link'
complete -c zc -n '__zc_using_command run' -s c -l compile -d 'Compile and assemble, but do not link'
complete -c zc -n '__zc_using_command run' -s n -l no-flags -d 'Do not add flags from configuration file'
complete -c zc -n '__zc_using_command run' -s r -l release -d 'Compile in release mode'
complete -c zc -n '__zc_using_command run' -l std -d 'Add C/C++ standard from config file'

# create
complete -c zc -n '__zc_using_command create' -s i -l input -d 'Files to use as basis' -r
complete -c zc -n '__zc_using_command create' -s f -l force -d 'Force creating file even if it already exists'
complete -c zc -n '__zc_using_command create' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command create' -s e -l edit -d 'Open files in editor once created'

# init
complete -c zc -n '__zc_using_command init' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command init' -s a -l author -d 'Package author' -x
complete -c zc -n '__zc_using_command init' -s t -l target -d 'Package target' -x
complete -c zc -n '__zc_using_command init' -s p -l project-template -d 'Project template to use' -x
complete -c zc -n '__zc_using_command init' -s n -l name -d 'Name of the package' -x
complete -c zc -n '__zc_using_command init' -s l -l languages -d 'Languages of the project' -x
complete -c zc -n '__zc_using_command init' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command init' -s f -l force -d 'Force initialization even if a project already exists'
complete -c zc -n '__zc_using_command init' -s g -l git -d 'Initialize empty git repository at project root'
complete -c zc -n '__zc_using_command init' -s e -l edit -d 'Open project in editor once initialized'
complete -c zc -n '__zc_using_command init' -s B -l bin -d 'Make package of type BIN'
complete -c zc -n '__zc_using_command init' -s L -l lib -d 'Make package of type LIB'
complete -c zc -n '__zc_using_command init' -s H -l header -d 'Make package of type HEADER'
complete -c zc -n '__zc_using_command init' -s C -l compose -d 'Make package of type COMPOSE'

# setup
complete -c zc -n '__zc_using_command setup' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command setup' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command setup' -s r -l release -d 'Create config for release mode'
complete -c zc -n '__zc_using_command setup' -s d -l debug -d 'Create config for debug mode'

# build
complete -c zc -n '__zc_using_command build' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command build' -s R -l run -d 'Run binary after compiling'
complete -c zc -n '__zc_using_command build' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command build' -s c -l clean -d 'Clean before building'
complete -c zc -n '__zc_using_command build' -s r -l release -d 'Build in release mode'
complete -c zc -n '__zc_using_command build' -s d -l debug -d 'Build in debug mode'

# add
complete -c zc -n '__zc_using_command add' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command add' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command add' -s s -l static -d 'Add dependency as static library'

# remove
complete -c zc -n '__zc_using_command remove' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command remove' -s q -l quiet -d 'Do not show any messages'

# use
complete -c zc -n '__zc_using_command use' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command use' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command use' -s g -l global -d 'Change default version of a package'

# publish
complete -c zc -n '__zc_using_command publish' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command publish' -s q -l quiet -d 'Do not show any messages'

# clean
complete -c zc -n '__zc_using_command clean' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command clean' -s q -l quiet -d 'Do not show any messages'

# list
complete -c zc -n '__zc_using_command list' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command list' -s d -l dependencies -d 'Show project dependencies'
complete -c zc -n '__zc_using_command list' -s t -l templates -d 'Show available templates instead of packages'
complete -c zc -n '__zc_using_command list' -s p -l project-templates -d 'Show available project templates instead of packages'
complete -c zc -n '__zc_using_command list' -s r -l remote -d 'Show remote packages instead of local ones'
complete -c zc -n '__zc_using_command list' -s s -l simple -d 'Use a simpler display'

# install
complete -c zc -n '__zc_using_command install' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command install' -s p -l path -d 'Install from local project instead of remote' -r
complete -c zc -n '__zc_using_command install' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command install' -s f -l force -d 'Force reinstalling packages'
complete -c zc -n '__zc_using_command install' -s s -l sync -d 'Also add installed packages to project dependencies'
complete -c zc -n '__zc_using_command install' -s S -l save-path -d 'Save the path of the local project in the registry'
complete -c zc -n '__zc_using_command install' -l std -d 'Add dependency to a standard library'

# uninstall
complete -c zc -n '__zc_using_command uninstall' -s q -l quiet -d 'Do not show any messages'

# update
complete -c zc -n '__zc_using_command update' -s P -l project-path -d 'Directory to use as project root' -r
complete -c zc -n '__zc_using_command update' -s p -l path -d 'Update local package from its root path' -r
complete -c zc -n '__zc_using_command update' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command update' -s f -l force -d 'Force reinstalling a specific version'
complete -c zc -n '__zc_using_command update' -s s -l sync -d 'Sync project dependencies after updating packages'
complete -c zc -n '__zc_using_command update' -s d -l dont-use -d 'Do not set newly installed version as default version'
complete -c zc -n '__zc_using_command update' -s S -l save-path -d 'Save the path of the local project in the registry'

# login
complete -c zc -n '__zc_using_command login' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command login' -s f -l force -d 'Force login even if an account is already logged in'

# logout
complete -c zc -n '__zc_using_command logout' -s q -l quiet -d 'Do not show any messages'

# config
complete -c zc -n '__zc_using_command config' -s q -l quiet -d 'Do not show any messages'
complete -c zc -n '__zc_using_command config' -s f -l force -d 'Override already existing configuration'

# languages
complete -c zc -n '__zc_using_command languages; and not __zc_using_subcommand languages add; and not __zc_using_subcommand languages remove; and not __zc_using_subcommand languages edit; and not __zc_using_subcommand languages show' -a 'add remove edit show'

complete -c zc -n '__zc_using_subcommand languages add' -s g -l global -d 'Modify global configuration'
complete -c zc -n '__zc_using_subcommand languages remove' -s g -l global -d 'Modify global configuration'
complete -c zc -n '__zc_using_subcommand languages edit' -s g -l global -d 'Modify global configuration'
complete -c zc -n '__zc_using_subcommand languages show' -s g -l global -d 'Show global configuration'
