/*
 * This file is part of Calligra project
 *
 * Copyright (c) 2016 Friedrich W. H. Kossebau <kossebau@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef KPLATOSCRIPTING_EXPORT_H_WRAPPER
#define KPLATOSCRIPTING_EXPORT_H_WRAPPER

/* This is the one which does the actual work, generated by cmake */
#include "kplatoscripting_generated_export.h"

/* Now the same for KPLATOSCRIPTING_TEST_EXPORT, if compiling with unit tests enabled */
#ifdef COMPILING_TESTS
#   define KPLATOSCRIPTING_TEST_EXPORT KPLATOSCRIPTING_EXPORT
#else /* not compiling tests */
#   define KPLATOSCRIPTING_TEST_EXPORT
#endif

#endif