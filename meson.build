project('bilimusic-gtk', 'c')

deps = [dependency('gtk4'), dependency('libadwaita-1'), dependency('libcjson'), dependency('libcurl'), dependency('webkitgtk-6.0')]

src = ['src/main.c', 'src/ui/ui_home.c', 'src/ui/ui_source.c', 'src/ui/ui_account.c', 'src/api_account.c']

executable('bilimusic-gtk', src, dependencies : deps)
