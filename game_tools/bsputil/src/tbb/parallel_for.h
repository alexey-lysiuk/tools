//
//---------------------------------------------------------------------------
//
// Copyright(C) 2017-2021 Alexey Lysiuk
// All rights reserved.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/
//
//--------------------------------------------------------------------------
//

#pragma once

#ifndef PARALLEL_FOR_H_INCLUDED
#define PARALLEL_FOR_H_INCLUDED

#ifdef HAVE_PARALLEL_FOR

#include <ppl.h>

namespace tbb
{
	template <typename Index, typename Function>
	inline void parallel_for(const Index first, const Index last, const Function& function)
	{
		concurrency::parallel_for(first, last, 1, function);
	}
}

#elif defined HAVE_DISPATCH_APPLY

#include <dispatch/dispatch.h>

namespace tbb
{
	template <typename Index, typename Function>
	inline void parallel_for(const Index first, const Index last, const Function& function)
	{
		const dispatch_queue_t queue = dispatch_get_global_queue(DISPATCH_QUEUE_PRIORITY_DEFAULT, 0);

		dispatch_apply(last - first, queue, ^(size_t slice)
		{
			function(slice);
		});
	}
}

#else // Generic loop with optional OpenMP parallelization

namespace tbb
{
	template <typename Index, typename Function>
	inline void parallel_for(const Index first, const Index last, const Function& function)
	{
#pragma omp parallel for
		for (Index i = first; i < last; ++i)
		{
			function(i);
		}
	}
}

#endif // HAVE_PARALLEL_FOR

#endif // PARALLEL_FOR_H_INCLUDED
