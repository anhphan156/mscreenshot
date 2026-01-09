project(
	'meme_screenshot',
	'c',
	license: 'MIT',
	meson_version: '>=0.59.0',
	default_options: ['c_std=c11', 'warning_level=2', 'werror=false'],
)

subdir('external/slurp')

cc = meson.get_compiler('c')
cairo = dependency('cairo')
realtime = cc.find_library('rt')
wayland_client = dependency('wayland-client')
wayland_cursor = dependency('wayland-cursor')
xkbcommon = dependency('xkbcommon')

executable(
  'meme_screenshot',
  ['src/main.c'],
  link_with: lib_slurp,
  dependencies: [
      cairo,
      realtime,
      wayland_client,
      wayland_cursor,
      xkbcommon,
  ],
)
