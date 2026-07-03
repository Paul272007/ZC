#!/usr/bin/env bash

_zc_completion() {
    local cur prev words cword split
    _init_completion -s || return

    local commands="run create init setup build add remove use publish clean list install uninstall update login logout config languages"
    local languages_commands="add remove edit show"

    case "${prev}" in
        -P|--project-path|-p|--path)
            _filedir -d
            return
            ;;
        -i|--input)
            _filedir "@(c|cc|cpp|cxx|C)"
            return
            ;;
        create|run)
            _filedir
            return
            ;;
    esac

    local command=""
    local subcommand=""
    
    # Find the main command
    for ((i=1; i<cword; i++)); do
        local word="${words[i]}"
        if [[ $commands == *"$word"* ]]; then
            command="$word"
            break
        fi
    done

    # If the command is 'languages', find its subcommand
    if [[ "$command" == "languages" ]]; then
        for ((j=i+1; j<cword; j++)); do
            local word="${words[j]}"
            if [[ $languages_commands == *"$word"* ]]; then
                subcommand="$word"
                break
            fi
        done
    fi

    # Handle global options if no command is specified yet
    if [[ -z "$command" ]]; then
        if [[ "$cur" == -* ]]; then
            COMPREPLY=( $(compgen -W "-v --version -h --help" -- "$cur") )
        else
            COMPREPLY=( $(compgen -W "$commands" -- "$cur") )
        fi
        return
    fi

    # Auto-completion per command
    if [[ "$cur" == -* ]]; then
        local opts=""
        case "$command" in
            run)       opts="-h --help -a --args -f --force -q --quiet -k --keep -p --plus -s --static -E --preprocess -S --assemble -c --compile -n --no-flags -r --release --std" ;;
            create)    opts="-h --help -i --input -f --force -q --quiet -e --edit" ;;
            init)      opts="-h --help -P --project-path -a --author -t --target -p --project-template -n --name -l --languages -q --quiet -f --force -g --git -e --edit -B --bin -L --lib -H --header -C --compose" ;;
            setup)     opts="-h --help -P --project-path -q --quiet -r --release -d --debug" ;;
            build)     opts="-h --help -P --project-path -R --run -q --quiet -c --clean -r --release -d --debug" ;;
            add)       opts="-h --help -P --project-path -q --quiet -s --static" ;;
            remove)    opts="-h --help -P --project-path -q --quiet" ;;
            use)       opts="-h --help -P --project-path -q --quiet -g --global" ;;
            publish)   opts="-h --help -P --project-path -q --quiet" ;;
            clean)     opts="-h --help -P --project-path -q --quiet" ;;
            list)      opts="-h --help -q --quiet -d --dependencies -t --templates -p --project-templates -r --remote -s --simple" ;;
            install)   opts="-h --help -P --project-path -p --path -q --quiet -f --force -s --sync -S --save-path --std" ;;
            uninstall) opts="-h --help -q --quiet" ;;
            update)    opts="-h --help -P --project-path -p --path -q --quiet -f --force -s --sync -d --dont-use -S --save-path" ;;
            login)     opts="-h --help -q --quiet -f --force" ;;
            logout)    opts="-h --help -q --quiet" ;;
            config)    opts="-h --help -q --quiet -f --force" ;;
            languages)
                if [[ -z "$subcommand" ]]; then
                    COMPREPLY=( $(compgen -W "$languages_commands -h --help" -- "$cur") )
                    return
                else
                    opts="-h --help -g --global"
                fi
                ;;
        esac
        COMPREPLY=( $(compgen -W "$opts" -- "$cur") )
    else
        case "$command" in
            languages)
                if [[ -z "$subcommand" ]]; then
                    COMPREPLY=( $(compgen -W "$languages_commands" -- "$cur") )
                fi
                ;;
            add|install)
                # Ideally fetch from 'zc list -r', but for now just fallback to default filedir or nothing
                ;;
            remove|use|update|uninstall)
                # Ideally fetch from 'zc list', but fallback
                ;;
        esac
    fi
}

complete -F _zc_completion zc
