#pragma once

// Place an ANALYZESTACK_ALIAS call directly before calling a function pointer to allow
// analyzestack.yaml annotation files for the stack analyzer to refer to the call by the alias,
// rather than by file and line number.
//
// Example usage:
//   ANALYZESTACK_ALIAS("test_alias")
//   some_struct->function_pointer();
// Then analyzestack.yaml files can annotate the function call like so:
//   add:
//     test_alias:
//       # my_function could be called at the aliased callsite.
//       - my_function

#define ANALYZESTACK_ALIAS(alias) __asm("__ANALYZESTACK_ALIAS$" alias ":");
