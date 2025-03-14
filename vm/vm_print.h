/*
 * Copyright (c) 2013 Free Software Foundation.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef VM_PRINT_H
#define	VM_PRINT_H

#include <vm/vm_map.h>
#include <machine/db_machdep.h>

/* Debugging: print a map */
extern void vm_map_print(db_expr_t addr, boolean_t have_addr,
                         db_expr_t count, const char *modif);

/* Pretty-print a copy object for ddb. */
extern void vm_map_copy_print(const vm_map_copy_t);

#include <vm/vm_object.h>

extern void vm_object_print_part(vm_object_t object, vm_offset_t offset, vm_size_t size);
extern void vm_object_print(vm_object_t);

#include <vm/vm_page.h>

extern void vm_page_print(const vm_page_t);

#endif	/* VM_PRINT_H */

