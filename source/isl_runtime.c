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

#include "acr/isl_runtime.h"

#include <assert.h>

#include <isl/constraint.h>
#include <isl/ctx.h>
#include <isl/space.h>
#include <isl/val.h>

isl_set** acr_isl_set_from_monitor(
    isl_ctx *ctx,
    struct acr_runtime_data *data_info,
    const unsigned char *data) {
    size_t num_alternatives = data_info->num_alternatives;
    unsigned int num_dimensions = data_info->num_monitor_dims;
    const size_t *dimensions = data_info->monitor_dim_max;
    size_t dimensions_total_size = data_info->monitor_total_size;
    size_t tiling_size = data_info->grid_size;
    struct runtime_alternative*
        (*const get_alternative_from_val)(unsigned char data) = data_info->alternative_from_val;

  isl_space *space = isl_space_set_alloc(ctx, 0, num_dimensions);
  isl_val *tiling_size_val = isl_val_int_from_ui(ctx, tiling_size);

  isl_set **sets = malloc(num_alternatives * sizeof(*sets));
  for (size_t i = 0; i < num_alternatives; ++i) {
    sets[i] = isl_set_empty(isl_space_copy(space));
  }
  size_t *current_dimension =
    calloc(num_dimensions, sizeof(*current_dimension));
  isl_local_space *local_space =
    isl_local_space_from_space(isl_space_copy(space));

  for(size_t i = 0; i < dimensions_total_size; ++i) {
    struct runtime_alternative *alternative = get_alternative_from_val(data[i]);
    assert(alternative != NULL);

    isl_set *tempset = isl_set_universe(isl_space_copy(space));
    for (unsigned int j = 0; j < num_dimensions; ++j) {
      size_t dimensions_pos = current_dimension[num_dimensions - 1 - j];
      isl_constraint *c_lower = isl_constraint_alloc_inequality(
          isl_local_space_copy(local_space));
      isl_constraint *c_upper = isl_constraint_copy(c_lower);
      isl_val *lower_bound =
        isl_val_mul_ui(isl_val_copy(tiling_size_val), dimensions_pos);
      lower_bound = isl_val_neg(lower_bound);
      isl_val *upper_bound =
        isl_val_mul_ui(isl_val_copy(tiling_size_val), dimensions_pos);
      upper_bound = isl_val_add(upper_bound, isl_val_copy(tiling_size_val));
      upper_bound = isl_val_sub_ui(upper_bound, 1);
      c_lower =
        isl_constraint_set_constant_val(c_lower, lower_bound);
      c_lower = isl_constraint_set_coefficient_si(c_lower, isl_dim_set, (int)j, 1);
      c_upper =
        isl_constraint_set_constant_val(c_upper, upper_bound);
      c_upper = isl_constraint_set_coefficient_si(c_upper, isl_dim_set, (int)j, -1);
      tempset = isl_set_add_constraint(tempset, c_lower);
      tempset = isl_set_add_constraint(tempset, c_upper);
    }
    sets[alternative->alternative_number] =
      isl_set_union(sets[alternative->alternative_number], tempset);

    for (size_t j = 0; j < num_dimensions; ++j) {
      current_dimension[j] += 1;
      if(current_dimension[j] == dimensions[j]) {
        current_dimension[j] = 0;
      } else {
        break;
      }
    }
  }

  isl_space_free(space);
  isl_local_space_free(local_space);
  isl_val_free(tiling_size_val);
  free(current_dimension);
  return sets;
}
