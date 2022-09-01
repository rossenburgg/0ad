/* Copyright (C) 2022 Wildfire Games.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "precompiled.h"

#include "lib/sysdep/os_cpu.h"
#include "lib/alignment.h"
#include "lib/bits.h"
#include "lib/config2.h"
#include "lib/module_init.h"

#if CONFIG2_VALGRIND
# include "valgrind.h"
#endif
#include <sys/param.h>
#include <sys/sysctl.h>

size_t os_cpu_NumProcessors()
{
	static size_t numProcessors;

	if(numProcessors == 0)
	{
#if CONFIG2_VALGRIND
		// Valgrind reports the number of real CPUs, but only emulates a single CPU.
		// That causes problems when we expect all those CPUs to be distinct, so
		// just pretend there's only one CPU
		if (RUNNING_ON_VALGRIND)
			numProcessors = 1;
		else
#endif
		{
			long res = sysconf(_SC_NPROCESSORS_CONF);
			ENSURE(res != -1);
			numProcessors = (size_t)res;
		}
	}

	return numProcessors;
}


uintptr_t os_cpu_ProcessorMask()
{
	static uintptr_t processorMask;

	if(!processorMask)
		processorMask = bit_mask<uintptr_t>(os_cpu_NumProcessors());

	return processorMask;
}


size_t os_cpu_PageSize()
{
	static size_t pageSize;

	if(!pageSize)
		pageSize = (size_t)sysconf(_SC_PAGESIZE);

	return pageSize;
}


size_t os_cpu_LargePageSize()
{
	// assume they're unsupported.
	return 0;
}


size_t os_cpu_QueryMemorySize()
{
	size_t memorySize = 0;
	size_t len = sizeof(memorySize);
	// Argh, the API doesn't seem to be const-correct
	/*const*/ int mib[2] = { CTL_HW, HW_PHYSMEM };
	sysctl(mib, 2, &memorySize, &len, nullptr, 0);
	memorySize /= MiB;
	return memorySize;
}


size_t os_cpu_MemoryAvailable()
{
	size_t memoryAvailable = 0;
	size_t len = sizeof(memoryAvailable);
	// Argh, the API doesn't seem to be const-correct
	/*const*/ int mib[2] = { CTL_HW, HW_USERMEM };
	sysctl(mib, 2, &memoryAvailable, &len, nullptr, 0);
	memoryAvailable /= MiB;
	return memoryAvailable;
}

uintptr_t os_cpu_SetThreadAffinityMask(uintptr_t UNUSED(processorMask))
{
	// not yet implemented
	return os_cpu_ProcessorMask();
}

Status os_cpu_CallByEachCPU(OsCpuCallback cb, uintptr_t cbData)
{
	for(size_t processor = 0; processor < os_cpu_NumProcessors(); processor++)
	{
		const uintptr_t processorMask = uintptr_t(1) << processor;
		os_cpu_SetThreadAffinityMask(processorMask);
		cb(processor, cbData);
	}

	return INFO::OK;
}
