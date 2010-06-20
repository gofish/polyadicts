
/*
** This file is part of PerTwisted, a persistant storage framework
** tailored around the Twisted asynchronous network programming framework,
** utilizing the Berkeley Database, Cononical's Storm ORM, implementations
** of various DHTs and hopefully much more.
**
** Pertwisted is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** PerTwisted is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** and the GNU Lesser Public License along with PerTwisted.  If not, see
** <http://www.gnu.org/licenses/>.
*/

#ifndef __ptmodule_h_DEFINED
#define __ptmodule_h_DEFINED

#include <Python.h>

PyMODINIT_FUNC init_pt (void);

#endif /* __ptmodule_h_DEFINED */
