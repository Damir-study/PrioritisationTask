#ifndef TREE_PRINTER_HPP
#define TREE_PRINTER_HPP

#include <ostream>
#include <string>
#include "../../external/mutable_array_sequence.h"

void print_tree_levels(std::ostream& out, const mutable_array_sequence<mutable_array_sequence<std::string>>& levels);

#endif // TREE_PRINTER_HPP
