hotelbookingreservation_sources = [
  'main.c',
  'hotelbookingreservation-window.c',
  'hotelbookingreservation-application.c',
]

hotelbookingreservation_deps = [
  dependency('libadwaita-1', version: '>= 1.0'),
]

gnome = import('gnome')

hotelbookingreservation_sources += gnome.compile_resources('hotelbookingreservation-resources',
  'hotelbookingreservation.gresource.xml',
  c_name: 'hotelbookingreservation'
)

executable('hotelbookingreservation', hotelbookingreservation_sources,
  dependencies: hotelbookingreservation_deps,
  install: true,
)
