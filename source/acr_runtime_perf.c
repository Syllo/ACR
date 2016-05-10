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

#include <dlfcn.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "acr/acr_runtime_build.h"
#include "acr/acr_runtime_perf.h"
#include "acr/acr_runtime_verify.h"
#include "acr/cloog_runtime.h"

#define GETCWD_DEFAULT_BUFFER_SIZE 100

struct acr_perf_compilation {
  size_t starting_at, ending_at;
  char *compilation_file_name;
  char *loop_body;
  unsigned char *monitor_result;
  void *handle;
  void *function;
};

struct acr_performance_list {
  struct acr_performance_list *next;
  struct acr_perf_compilation element;
};

static void acr_getcwd(char **cwd, size_t *buffer_size) {
  *buffer_size = GETCWD_DEFAULT_BUFFER_SIZE;
  char *file = malloc(*buffer_size * sizeof(file));
  char *pwd;
  while ((pwd = getcwd(file, *buffer_size)) == NULL) {
    if (errno == ERANGE) {
      *buffer_size += GETCWD_DEFAULT_BUFFER_SIZE;
      file = realloc(file, *buffer_size);
    } else {
      perror("getcwd");
      exit(EXIT_FAILURE);
    }
  }
  *cwd = file;
}

static void acr_perf_add_compilation_to_list(struct acr_runtime_perf *perf,
    struct acr_perf_compilation compilation) {
  struct acr_performance_list *plist = malloc(sizeof(*plist));
  plist->element = compilation;
  plist->next = NULL;
  if (perf->compilation_list == NULL) {
    perf->compilation_list = plist;
  } else {
    perf->list_head->next = plist;
  }
  perf->list_head = plist;
}

static void perf_new_compilation(struct acr_runtime_perf *perf,
                                 unsigned char *monitor_result) {

  size_t perf_step = perf->list_head == NULL ? 0 : perf->list_head->element.ending_at + 1;
  char *generated_function = NULL;
  size_t generated_function_size;
  char *body = NULL;
  FILE *bodystream = open_memstream(&body, &generated_function_size);
  acr_cloog_generate_alternative_code_from_input(bodystream, perf->rdata,
      monitor_result);
  fclose(bodystream);
  FILE *memstring = open_memstream(&generated_function, &generated_function_size);
  fprintf(memstring, "void acr_function%s {\n%s}\n",
      perf->rdata->function_prototype, body);
  fclose(memstring);

  char *filename = acr_compile_with_system_compiler(
      generated_function,
      perf->rdata->num_compiler_flags, perf->rdata->compiler_flags);
  free(generated_function);

  struct acr_perf_compilation comp;
  comp.compilation_file_name = filename;
  comp.handle = dlopen(filename, RTLD_NOW);
  if (comp.handle == NULL) {
    perror("dlopen");
    exit(EXIT_FAILURE);
  }
  unlink(filename);
  comp.function = dlsym(comp.handle, "acr_function");
  if (comp.function == NULL) {
    perror("dlsym");
    exit(EXIT_FAILURE);
  }
  comp.starting_at = perf_step;
  comp.ending_at = perf_step;
  comp.monitor_result = monitor_result;
  comp.loop_body = body;
  acr_perf_add_compilation_to_list(perf, comp);
}

static void acr_perf_compilation_clean(struct acr_perf_compilation *comp) {
  free(comp->loop_body);
  free(comp->compilation_file_name);
  free(comp->monitor_result);
  dlclose(comp->handle);
}

void acr_runtime_perf_clean(struct acr_runtime_perf *perf) {
  struct acr_performance_list *plist = perf->compilation_list;
  struct acr_performance_list *plist_temp;
  char *cwd;
  size_t buffer_size;
  acr_getcwd(&cwd, &buffer_size);
  size_t path_size = strlen(cwd);
  if (path_size + 17 > buffer_size) {
    cwd = realloc(cwd, buffer_size + 17);
  }
  memcpy(&cwd[path_size], "/acr_perf_XXXXXX", 17);
  int fd = mkstemp(cwd);
  if (fd == -1) {
    perror("mkstemp");
    exit(EXIT_FAILURE);
  }
  FILE *output_file = fdopen(fd, "w");
  acr_runtime_print_perf_function_call(output_file, perf);
  fclose(output_file);
  fprintf(stdout, "\nACR perf function written in file %s\n", cwd);
  while (plist) {
    plist_temp = plist->next;
    acr_perf_compilation_clean(&plist->element);
    free(plist);
    plist = plist_temp;
  }
  free(cwd);
}

void* acr_runntime_perf_end_step(struct acr_runtime_perf *perf) {
  unsigned char *monitor_result =
    malloc(perf->rdata->monitor_total_size * sizeof(*monitor_result));
  perf->rdata->monitoring_function(monitor_result);
  if (perf->list_head &&
      acr_verify_me(perf->rdata->monitor_total_size,
        perf->list_head->element.monitor_result,
        monitor_result)) {
      free(monitor_result);
      perf->list_head->element.ending_at +=1;
      return perf->list_head->element.function;
  }
  // Generate new function
  perf_new_compilation(perf, monitor_result);
  perf->rdata->current_monitoring_data = perf->list_head->element.monitor_result;
  return perf->list_head->element.function;
}

void acr_runtime_print_perf_function_call(
    FILE* output,
    struct acr_runtime_perf *perf) {
  struct acr_performance_list *list_element = perf->compilation_list;
  size_t num_functions = 0;
  while (list_element) {
    num_functions += 1;
    list_element = list_element->next;
  }
  fprintf(output, "#include <stdlib.h>\nvoid acr_perf_function%s {\n",
      perf->rdata->function_prototype);
  list_element = perf->compilation_list;
  while (list_element) {
    if (list_element->element.starting_at != list_element->element.ending_at) {
      fprintf(output, "for(size_t i = 0; i < %zu; ++i) {\n",
          1+list_element->element.ending_at - list_element->element.starting_at);
    }
    fprintf(output, "{%s}", list_element->element.loop_body);
    if (list_element->element.starting_at != list_element->element.ending_at)
      fprintf(output, "}\n");
    list_element = list_element->next;
  }
  fprintf(output, "}\n");
}
