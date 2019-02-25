// Copyright 2018 Google Inc.  All rights reserved
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#ifndef GOOGLE_API_CODEGEN_GAPIC_TEMPLATE_H_
#define GOOGLE_API_CODEGEN_GAPIC_TEMPLATE_H_

#include <memory>

namespace google {
namespace api {
namespace codegen {
namespace internal {

class GapicTemplate {
 public:
  virtual ~GapicTemplate() = default;

  virtual char const* include() const = 0;
  virtual char const* namespaceAlias() const = 0;
  virtual char const* namespaceStart() const = 0;
  virtual char const* namespaceEnd() const = 0;

  virtual char const* headerFileStart() const = 0; 
  virtual char const* headerFileEnd() const = 0; 
  virtual char const* headerGuardStart() const = 0;
  virtual char const* headerGuardEnd() const = 0;
  virtual char const* headerKnownIncludes() const = 0;
  virtual char const* headerPreClient() const = 0;
  virtual char const* headerPostClient() const = 0;
  virtual char const* headerClientStart() const = 0;
  virtual char const* headerClientEnd() const = 0;
  virtual char const* headerClientPublic() const = 0;
  virtual char const* headerClientPublicMethod() const = 0;
  virtual char const* headerClientProtected() const = 0;
  virtual char const* headerClientProtectedMethod() const = 0;
  virtual char const* headerClientPrivate() const = 0;
  virtual char const* headerClientPrivateMethod() const = 0;
};

std::shared_ptr<GapicTemplate> CreateDefaultGapicTemplate();

} // namespace internal
} // namespace codegen
} // namespace api
} // namespace google

#endif // GOOGLE_API_CODEGEN_GAPIC_TEMPLATE_H_
