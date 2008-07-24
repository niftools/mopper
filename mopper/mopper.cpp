// ***** BEGIN LICENSE BLOCK *****
//
// Copyright (c) 2006-2008, NIF File Format Library and Tools.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    * Redistributions of source code must retain the above copyright
//      notice, this list of conditions and the following disclaimer.
//
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided
//      with the distribution.
//
//    * Neither the name of the NIF File Format Library and Tools
//      project nor the names of its contributors may be used to endorse
//      or promote products derived from this software without specific
//      prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// ***** END LICENSE BLOCK *****

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <cstring>

// Math and base include
#include <Common/Base/hkBase.h>
#include <Common/Base/System/hkBaseSystem.h>
#include <Common/Base/Memory/hkThreadMemory.h>
#include <Common/Base/Memory/Memory/Pool/hkPoolMemory.h>
#include <Common/Base/System/Error/hkDefaultError.h>
#include <Common/Base/Monitor/hkMonitorStream.h>

#include <Common/Base/System/Io/FileSystem/hkFileSystem.h>
#include <Common/Base/Container/LocalArray/hkLocalBuffer.h>
//
#include <Physics/Collide/Shape/Convex/Box/hkpBoxShape.h>
#include <Physics/Collide/Shape/Convex/ConvexTranslate/hkpConvexTranslateShape.h>
#include <Physics/Collide/Shape/Convex/ConvexTransform/hkpConvexTransformShape.h>
#include <Physics/Collide/Shape/Compound/Collection/SimpleMesh/hkpSimpleMeshShape.h>
#include <Physics/Collide/Shape/Compound/Collection/List/hkpListShape.h>
#include <Physics/Collide/Shape/Convex/Capsule/hkpCapsuleShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppBvTreeShape.h>
#include <Physics/Collide/Shape/Compound/Tree/Mopp/hkpMoppUtility.h>
#include <Physics/Internal/Collide/Mopp/Code/hkpMoppCode.h>

#pragma comment(lib, "hkBase.lib")
#pragma comment(lib, "hkSerialize.lib")
#pragma comment(lib, "hkpInternal.lib")
#pragma comment(lib, "hkpUtilities.lib")
#pragma comment(lib, "hkpCollide.lib")
#pragma comment(lib, "hkpConstraintSolver.lib")

static void HK_CALL errorReport(const char* msg, void*)
{
	std::cout << msg;
}

hkpMoppCode *buildCode(const hkpSimpleMeshShape * list, const hkpMoppCompilerInput * mfr) {
	__try
	{
		return hkpMoppUtility::buildCode(list, *mfr);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

void mopper(std::istream & infile) {
	hkpSimpleMeshShape * list = NULL;
	hkpMoppCode* k_phkpMoppCode = NULL;

	//std::cout << "info: building mesh" << std::endl;
	list = new hkpSimpleMeshShape( 0.01f );
	hkArray<hkVector4> &vertices = list->m_vertices;
	hkArray<hkpSimpleMeshShape::Triangle> &triangles = list->m_triangles;

	vertices.setSize(0);
	triangles.setSize(0);

	int numvertices;
	infile >> numvertices;
	//std::cout << "info: " << numvertices << " vertices" << std::endl;
	for (int i = 0; i < numvertices; i++) {
		float x, y, z;
		infile >> x >> y >> z;
		if (infile.eof() || infile.fail()) {
			//std::cout
			//	<< "info: error while parsing vertices"
			//	<< std::endl;
			return;
		}
		//std::cout
		//	<< "info: adding vertex "
		//	<< x << ", "
		//	<< y << ", "
		//	<< z << std::endl;
		vertices.pushBack( hkVector4(x, y, z) );
	}

	int numtriangles;
	infile >> numtriangles;
	//std::cout << "info: " << numtriangles << " triangles" << std::endl;
	for (int i = 0; i < numtriangles; i++) {
		hkpSimpleMeshShape::Triangle hktri;
		infile >> hktri.m_a >> hktri.m_b >> hktri.m_c;
		if (infile.eof() || infile.fail()) {
			//std::cout
			//	<< "info: error while parsing triangles"
			//	<< std::endl;
			return;
		}
		if (infile.fail())
			break;
		//std::cout
		//	<< "info: adding triangle "
		//	<< hktri.m_a << ", "
		//	<< hktri.m_b << ", "
		//	<< hktri.m_c << std::endl;
		triangles.pushBack( hktri );
	}


	//std::cout << "info: building mopp in progress" << std::endl;
	hkpMoppCompilerInput mfr;
	mfr.setAbsoluteFitToleranceOfAxisAlignedTriangles( hkVector4( 0.1f, 0.1f, 0.1f ) );
	//mfr.setAbsoluteFitToleranceOfTriangles(0.1f);
	//mfr.setAbsoluteFitToleranceOfInternalNodes(0.0001f);
	k_phkpMoppCode = buildCode(list, &mfr);

	if (k_phkpMoppCode != NULL) {
		//std::cout << "info: building mopp finished" << std::endl;
		//std::cout
		//	<< "info: mopp size is "
		//	<< k_phkpMoppCode->m_data.getSize() << std::endl;

		// print mopp
		std::cout.precision(16);
		std::cout << k_phkpMoppCode->m_info.m_offset(0) << std::endl;
		std::cout << k_phkpMoppCode->m_info.m_offset(1) << std::endl;
		std::cout << k_phkpMoppCode->m_info.m_offset(2) << std::endl;
		std::cout << k_phkpMoppCode->m_info.getScale() << std::endl;
		std::cout << k_phkpMoppCode->m_data.getSize() << std::endl;
		for (int i = 0; i < k_phkpMoppCode->m_data.getSize(); i++) {
			std::cout
				<< int(k_phkpMoppCode->m_data[i])
				<< std::endl;
		}
	} else {
		//std::cout << "info: building mopp failed" << k_phkpMoppCode->m_data.getSize() << std::endl;
	}

	// Deallocate shape
	if (list) {
		list->removeReference();
		list = NULL;
	}

	// Deallocate mopp code
	if (k_phkpMoppCode)
	{
		k_phkpMoppCode->removeReference();
		k_phkpMoppCode = NULL;
	}
}

//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char *argv[])
{
	if (argc != 2) {
		std::cout
			<< "Mopper. Copyright (c) 2008, NIF File Format Library and Tools." << std::endl
			<< "All rights reserved." << std::endl
			<< std::endl
			<< "Options:" << std::endl
			<< "  --help      for usage help" << std::endl
			<< "  --license   for licensing details" << std::endl
			<< std::endl
			<< "Mopper uses havok. "
			<< "Copyright 1999-2008 Havok.com Inc. (and its Licensors)."
			<< std::endl
			<< "All Rights Reserved. See www.havok.com for details."
			<< std::endl << std::endl;
		return 0;
	}

	if (std::strcmp(argv[1], "--help") == 0) {
		std::cout <<
"usage: mopper.exe [<file>|--]\n\n"
"where <file> (-- for standard input) is of the following format:\n"
"<number of vertices>\n"
"<vertex 1 x> <vertex 1 y> <vertex 1 z>\n"
"<vertex 2 x> <vertex 2 y> <vertex 2 z>\n"
"...\n"
"<number of triangles>\n"
"<triangle 1 index 0> <triangle 1 index 1> <triangle 1 index 2>\n"
"<triangle 1 index 0> <triangle 2 index 1> <triangle 2 index 2>\n"
"...\n\n";
		return 0;
	}

	if (std::strcmp(argv[1], "--license") == 0) {
		std::cout <<
"Mopper. Copyright (c) 2008, NIF File Format Library and Tools\n"
"All rights reserved.\n\n"
"Redistribution and use in source and binary forms, with or without\n"
"modification, are permitted provided that the following conditions\n"
"are met:\n"
"1. Redistributions of source code must retain the above copyright\n"
"   notice, this list of conditions and the following disclaimer.\n"
"2. Redistributions in binary form must reproduce the above copyright\n"
"   notice, this list of conditions and the following disclaimer in the\n"
"   documentation and/or other materials provided with the distribution.\n"
"3. The name of the NIF File Format Library and Tools projectmay not be\n"
"   used to endorse or promote products derived from this software\n"
"   without specific prior written permission.\n"
"\n"
"THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
"IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n"
"OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n"
"IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
"INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n"
"NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n"
"THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n";
		return 0;
	}

	//std::cout << "info: initializing havok" << std::endl;
	// Initialize the base system including our memory system
	hkThreadMemory* threadMemory = NULL;
	char* stackBuffer = NULL;
	hkPoolMemory* memoryManager = new hkPoolMemory();
	threadMemory = new hkThreadMemory(memoryManager, 16);
	hkBaseSystem::init( memoryManager, threadMemory, errorReport );
	memoryManager->removeReference();

	// We now initialize the stack area to 100k (fast temporary memory to be used by the engine).
	int stackSize = 0x100000;
	stackBuffer = hkAllocate<char>( stackSize, HK_MEMORY_CLASS_BASE);
	hkThreadMemory::getInstance().setStackArea( stackBuffer, stackSize);

	// Call main program.
	if (std::strcmp(argv[1], "--") == 0) {
		mopper(std::cin);
	} else {
		mopper(std::ifstream(argv[1], std::ifstream::in));
	}

	//std::cout << "info: closing havok" << std::endl;

	// Deallocate stack area
	if (threadMemory)
	{
		threadMemory->setStackArea(0, 0);
		hkDeallocate(stackBuffer);

		threadMemory->removeReference();
		threadMemory = NULL;
		stackBuffer = NULL;
	}

	// Quit base system
	hkBaseSystem::quit();

	return 0;
}

