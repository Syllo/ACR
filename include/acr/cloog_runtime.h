/*
 * Copyright (C) 2016 Maxime Schmitt
 *
 * ACR is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef __ACR_CLOOG_RUNTIME_H
#define __ACR_CLOOG_RUNTIME_H

#include <acr/acr_runtime_data.h>
#include <acr/runtime_alternatives.h>
#include <isl/set.h>

void acr_cloog_generate_alternative_code_from_input(
    FILE* output,
    struct acr_runtime_data *data_info,
    const unsigned char *data,
    struct runtime_alternative*
        (*get_alternative_from_val)(unsigned char data));

void acr_cloog_init_alternative_constraint_from_cloog_union_domain(
    struct acr_runtime_data *data);

#endif // __ACR_CLOOG_RUNTIME_H
