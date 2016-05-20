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

/**
 *
 * \file runtime_alternative.h
 * \brief Structure to store alternative data
 *
 * \defgroup runtime_alternative
 *
 * @{
 * \brief Structure storing the alternative data required during runtime
 *
 */

#ifndef __ACR_RUNTIME_ALTERNATIVES__H
#define __ACR_RUNTIME_ALTERNATIVES__H

#include <stdint.h>
#include <isl/set.h>

enum acr_runtime_alternative_type {
  acr_runtime_alternative_parameter,
  acr_runtime_alternative_function,
};

struct runtime_alternative {
  isl_set **restricted_domains;
  size_t alternative_number;
  struct {
    union {
      char *function_to_swap;
      struct {
        unsigned int parameter_id;
        intmax_t parameter_value;
        void *function_matching_alternative;
      } parameter;
    } alt;
    char *name_to_swap;
  } value;
  enum acr_runtime_alternative_type type;
};

#endif // __ACR_RUNTIME_ALTERNATIVES__H

/**
 *
 * @}
 *
 */
