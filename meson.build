project('bilimusic-gtk', 'c')

deps = [dependency('gtk4'), dependency('libadwaita-1'), dependency('libcjson'), dependency('libcurl'), dependency('webkitgtk-6.0')]

src = ['src/main.c', 'src/ui/home.c', 'src/ui/source.c', 'src/ui/login.c', 'src/api/account.c']

executable('bilimusic-gtk', src, dependencies : deps)
