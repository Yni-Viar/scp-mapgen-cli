env = Environment()
is_msvc: bool = 'msvc' in env['TOOLS']
# Used from Godot Engine.

# Godot is licensed under MIT License

# Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md).
# Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Set our C and C++ standard requirements.
if not is_msvc:
    # Specifying GNU extensions support explicitly, which are supported by
    # both GCC and Clang.
    env.Prepend(CXXFLAGS=["-std=gnu++20"])
else:
    # MSVC started offering C standard support with Visual Studio 2019 16.8, which covers all
    # of our supported VS2019 & VS2022 versions; VS2017 will only pass the C++ standard.
    env.Prepend(CXXFLAGS=["/std:c++20"])
    # MSVC is non-conforming with the C++ standard by default, so we enable more conformance.
    # Note that this is still not complete conformance, as certain Windows-related headers
    # don't compile under complete conformance.
    env.Prepend(CCFLAGS=["/permissive-"])
    # Allow use of `__cplusplus` macro to determine C++ standard universally.
    env.Prepend(CXXFLAGS=["/Zc:__cplusplus"])

# Godot code end.

env.Program('mapgen', ['MapGenCore.cpp'])