/*
Copyright 2020 The OneFlow Authors. All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/
#include "oneflow/api/python/functional/python_arg_parser.h"
#include "oneflow/api/python/functional/common.h"
#include "oneflow/api/python/functional/python_arg.h"

namespace oneflow {
namespace one {
namespace functional {

void FunctionSchema::ReportKwargsError(PyObject* kwargs) const {
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  while (PyDict_Next(kwargs, &pos, &key, &value)) {
    if (!PyStringCheck(key)) { THROW(TypeError) << def_->name << "(): keywords must be strings."; }
    bool unexpected_param = true;
    const std::string string_key = PyStringAsString(key);
    for (const auto& arg : def_->argument_def) {
      if (arg.name == string_key) {
        unexpected_param = false;
        break;
      }
    }
    if (unexpected_param) {
      THROW(TypeError) << def_->name  // NOLINT
                       << "(): got an unexpected keyword argument '" << string_key << "'";
    } else {
      THROW(TypeError) << def_->name  // NOLINT
                       << "(): got multiple values for argument '" << string_key << "'";
    }
  }
  THROW(TypeError) << def_->name << "(): kwargs unknown error.";
}

// The argument parsing refers to the implementation of Pytorch.
bool FunctionSchema::Parse(PyObject* args, PyObject* kwargs, PythonArg* parsed_args,
                           bool raise_exception) const {
  bool treat_args_as_list = false;
  size_t nargs = args ? PyTuple_Size(args) : 0;
  size_t remaining_kwargs = kwargs ? PyDict_Size(kwargs) : 0;

  if (max_pos_nargs_ == 1) {
    const auto& type = def_->argument_def.at(0).type;
    treat_args_as_list = IsIntegralListType(type) || type == kSHAPE || type == kTENSOR_TUPLE;
  }
  if (nargs > max_pos_nargs_ && !treat_args_as_list) {
    if (raise_exception) {
      THROW(TypeError) << def_->name << "(): takes " << max_pos_nargs_
                       << " positional arguments but " << nargs << " were given.";
    }
    return false;
  }
  int arg_pos = 0;
  for (int i = 0; i < def_->argument_def.size(); ++i) {
    const auto& param = def_->argument_def.at(i);
    PyObject* obj = NULL;
    if (args && arg_pos < nargs) {
      if (param.keyword_only) {
        if (raise_exception) {
          THROW(TypeError) << def_->name << "(): argument '" << param.name << "' is keyword only.";
        }
        return false;
      }
      obj = PyTuple_GetItem(args, arg_pos);
    } else if (kwargs) {
      obj = PyDict_GetItemString(kwargs, param.name.c_str());
      if (obj) { remaining_kwargs--; }
    }

    if (obj) {
      if (arg_pos == 0 && treat_args_as_list && !param.keyword_only
          && (PyLong_Check(obj) || PyTensor_Check(obj))) {
        obj = args;
        arg_pos = nargs;
      } else {
        arg_pos++;
      }
      PythonArg arg(obj, param.size);
      if ((obj == Py_None && param.optional) || PythonArgCheck(arg, param.type)) {
        parsed_args[i] = std::move(arg);
      } else {
        if (raise_exception) {
          THROW(TypeError) << def_->name << "(): argument '" << param.name << "' must be "
                           << ValueTypeName(param.type) << ", not "
                           << PyStringAsString(PyObject_Str((PyObject*)Py_TYPE(obj)));
        }
        return false;
      }
    } else {
      if (!param.has_default_value) {
        if (raise_exception) {
          THROW(TypeError) << def_->name << "(): missing required argument " << param.name;
        }
        return false;
      }
      parsed_args[i] = param.default_value;
    }
  }
  if (remaining_kwargs > 0) {
    if (raise_exception) { ReportKwargsError(kwargs); }
    return false;
  }
  return true;
}

}  // namespace functional
}  // namespace one
}  // namespace oneflow