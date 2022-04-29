project('pact', 'c')
common_srcs = files(
  'src/chunk.c',
  #'src/debug.c',
  'src/memory.c',
  'src/object.c',
  'src/table.c',
  'src/value.c',
  'src/vm.c',
)
compiler_srcs = files(
  'src/scanner.c',
  'src/compiler.c'
)
interpreter = executable('pact', 'bins/interpreter.c', compiler_srcs, common_srcs)
compiler = executable('pactc', 'bins/compiler.c', compiler_srcs, common_srcs)
vm = executable('pactvm', 'bins/vm.c', common_srcs, c_args: ['-DVM_ONLY'])
