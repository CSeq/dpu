
/* 
 * Copyright (C) 2010, 2011  Cesar Rodriguez <cesar.rodriguez@lsv.ens-cachan.fr>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
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
 */


/* see src/verbosity.h for more info */
#define CONFIG_MAX_VERB_LEVEL 3

/* test and debug */
#define CONFIG_DEBUG

#define CONFIG_MAX_PROCESSES 32
#define CONFIG_MAX_EVENTS_PER_PROCCESS 30000

#define CONFIG_GUEST_MEMORY_SIZE (128 << 20)
#define CONFIG_GUEST_THREAD_STACK_SIZE (16 << 20)
#define CONFIG_GUEST_TRACE_BUFFER_SIZE (16 << 20)

