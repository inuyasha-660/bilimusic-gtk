project('bilimusic-gtk', 'c')

deps = [dependency('gtk4'), dependency('libadwaita-1'), dependency('libcjson'), dependency('libcurl'), dependency('webkitgtk-6.0')]
src = files(
  'src/main.c',
  'src/ui/home.c',
  'src/ui/login.c',
  'src/ui/source.c',
  'src/api/get_music.c',
  'src/api/import.c',
  'src/api/import_manually.c',
  'src/api/login_get_info.c',
  'src/api/start_get_account.c',
  'src/api/start_read_list.c',
  'src/api/start_get_favo.c',
  'src/api/utils.c',
  )
link_args = ['-lm']

executable('bilimusic-gtk', src, link_args: link_args, dependencies : deps)
