project('bilimusic-gtk', 'c')

deps = [dependency('gtk4'), dependency('libadwaita-1'), dependency('libcjson'), dependency('libcurl'), dependency('webkitgtk-6.0')]
src = ['src/main.c', 'src/ui/home.c', 'src/ui/source.c', 'src/ui/login.c', 'src/api/account.c']
link_args = ['-lm']

executable('bilimusic-gtk', src, link_args: link_args, dependencies : deps)
