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

#include "acr/acr_runtime_data.h"
#include <acr/acr_runtime_data.h>

void free_acr_runtime_data(struct acr_runtime_data* data) {
  cloog_input_free(data->cloog_input);
  cloog_state_free(data->state);
  osl_scop_free(data->osl_relation);
}

void init_acr_runtime_data(
    struct acr_runtime_data* data,
    char *scop,
    size_t scop_size) {
  data->osl_relation = acr_read_scop_from_buffer(scop, scop_size);
  data->state = cloog_state_malloc();
  data->cloog_input = cloog_input_from_osl_scop(data->state,
      data->osl_relation);
}