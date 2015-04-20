@echo off
pandoc api_python.md -o api_python.html -s -S -t html5 -p --normalize --toc --self-contained -f markdown+lists_without_preceding_blankline+compact_definition_lists-blank_before_header