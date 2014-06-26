// This code contains NVIDIA Confidential Information and is disclosed to you
// under a form of NVIDIA software license agreement provided separately to you.
//
// Notice
// NVIDIA Corporation and its licensors retain all intellectual property and
// proprietary rights in and to this software and related documentation and
// any modifications thereto. Any use, reproduction, disclosure, or
// distribution of this software and related documentation without an express
// license agreement from NVIDIA Corporation is strictly prohibited.
//
// ALL NVIDIA DESIGN SPECIFICATIONS, CODE ARE PROVIDED "AS IS.". NVIDIA MAKES
// NO WARRANTIES, EXPRESSED, IMPLIED, STATUTORY, OR OTHERWISE WITH RESPECT TO
// THE MATERIALS, AND EXPRESSLY DISCLAIMS ALL IMPLIED WARRANTIES OF NONINFRINGEMENT,
// MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE.
//
// Information and code furnished is believed to be accurate and reliable.
// However, NVIDIA Corporation assumes no responsibility for the consequences of use of such
// information or for any infringement of patents or other rights of third parties that may
// result from its use. No license is granted by implication or otherwise under any patent
// or patent rights of NVIDIA Corporation. Details are subject to change without notice.
// This code supersedes and replaces all information previously supplied.
// NVIDIA Corporation products are not authorized for use as critical
// components in life support devices or systems without express written approval of
// NVIDIA Corporation.
//
// Copyright (c) 2008-2013 NVIDIA Corporation. All rights reserved.
// Copyright (c) 2004-2008 AGEIA Technologies, Inc. All rights reserved.
// Copyright (c) 2001-2004 NovodeX AG. All rights reserved.


#ifndef NV_FOUNDATION_INTERFACE_H
#define NV_FOUNDATION_INTERFACE_H

#include "NvFoundation.h"

#ifndef NV_DOXYGEN
namespace nvidia
{
#endif


//***************************************
// FILE: NvErrors.h
//***************************************

/**
\brief Error codes

These error codes are passed to #NvErrorCallback

@see NvErrorCallback
*/

struct NvErrorCode
{
	enum Enum
	{
		eNO_ERROR				= 0,

		//! \brief An informational message.
		eDEBUG_INFO				= 1,

		//! \brief a warning message for the user to help with debugging
		eDEBUG_WARNING			= 2,

		//! \brief method called with invalid parameter(s)
		eINVALID_PARAMETER		= 4,

		//! \brief method was called at a time when an operation is not possible
		eINVALID_OPERATION		= 8,

		//! \brief method failed to allocate some memory
		eOUT_OF_MEMORY			= 16,

		/** \brief The library failed for some reason.
		Possibly you have passed invalid values like NaNs, which are not checked for.
		*/
		eINTERNAL_ERROR			= 32,

		//! \brief An unrecoverable error, execution should be halted and log output flushed
		eABORT					= 64,

		//! \brief The SDK has determined that an operation may result in poor performance.
		ePERF_WARNING			= 128,

		//! \brief A bit mask for including all errors
		eMASK_ALL				= -1
	};
};



//***************************************
// FILE: NvAllocatorCallback.h
//***************************************

/**
\brief Abstract base class for an application defined memory allocator that can be used by the Nv library.

\note The SDK state should not be modified from within any allocation/free function.

<b>Threading:</b> All methods of this class should be thread safe as it can be called from the user thread
or the physics processing thread(s).
*/

class NvAllocatorCallback
{
public:

	/**
	\brief destructor
	*/
	virtual ~NvAllocatorCallback() {}

	/**
	\brief Allocates size bytes of memory, which must be 16-byte aligned.

	This method should never return NULL.  If you run out of memory, then
	you should terminate the app or take some other appropriate action.

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread
	and physics processing thread(s).

	\param size			Number of bytes to allocate.
	\param typeName		Name of the datatype that is being allocated
	\param filename		The source file which allocated the memory
	\param line			The source line which allocated the memory
	\return				The allocated block of memory.
	*/
	virtual void* allocate(size_t size, const char* typeName, const char* filename, int line) = 0;

	/**
	\brief Frees memory previously allocated by allocate().

	<b>Threading:</b> This function should be thread safe as it can be called in the context of the user thread
	and physics processing thread(s).

	\param ptr Memory to free.
	*/
	virtual void deallocate(void* ptr) = 0;
};

//********************************************
// FILE: NvErrorCallback.h
//********************************************


/**
\brief User defined interface class.  Used by the library to emit debug information.

\note The SDK state should not be modified from within any error reporting functions.

<b>Threading:</b> The SDK sequences its calls to the output stream using a mutex, so the class need not
be implemented in a thread-safe manner if the SDK is the only client.
*/
class NvErrorCallback
{
public:

	virtual ~NvErrorCallback() {}

	/**
	\brief Reports an error code.
	\param code Error code, see #NvErrorCode
	\param message Message to display.
	\param file File error occured in.
	\param line Line number error occured on.
	*/
	virtual void reportError(NvErrorCode::Enum code, const char* message, const char* file, int line) = 0;

};

// *******************************************
// FILE: NvIO.h
// *******************************************

/**
\brief Input stream class for I/O.

The user needs to supply a NvInputStream implementation to a number of methods to allow the SDK to read data.
*/



class NvInputStream
{
public:

	/**
	\brief read from the stream. The number of bytes written may be less than the number requested.

	\param[in] dest the destination address to which the data will be written
	\param[in] count the number of bytes requested

	\return the number of bytes read from the stream.
	*/

	virtual uint32_t read(void* dest, uint32_t count) = 0;

	virtual ~NvInputStream() {}
};


/**
\brief Input data class for I/O which provides random read access.

The user needs to supply a NvInputData implementation to a number of methods to allow the SDK to read data. 
*/

class NvInputData : public NvInputStream
{
public:

	/**
	\brief return the length of the input data

	\return size in bytes of the input data
	*/

	virtual uint32_t	getLength() const			= 0;


	/**
	\brief seek to the given offset from the start of the data. 
	
	\param[in] offset the offset to seek to. 	If greater than the length of the data, this call is equivalent to seek(length);
	*/

	virtual void	seek(uint32_t offset)		= 0;

	/**
	\brief return the current offset from the start of the data
	
	\return the offset to seek to.
	*/

	virtual uint32_t	tell() const			= 0;

	virtual ~NvInputData() {}
};

/**
\brief Output stream class for I/O.

The user needs to supply a NvOutputStream implementation to a number of methods to allow the SDK to write data. 
*/

class NvOutputStream
{
public:
	/**
	\brief write to the stream. The number of bytes written may be less than the number sent.

	\param[in] src the destination address from which the data will be written
	\param[in] count the number of bytes to be written

	\return the number of bytes written to the stream by this call.
	*/

	virtual uint32_t write(const void* src, uint32_t count) = 0;

	virtual ~NvOutputStream() {}

};



#ifndef NV_DOXYGEN
} // namespace nvidia
#endif

/** @} */


#endif // NV_FOUNDATION_H
