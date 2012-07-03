// ***** BEGIN LICENSE BLOCK *****
//
// Copyright (c) 2006-2012, NIF File Format Library and Tools.
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
#include <Common/Base/Memory/System/Util/hkMemoryInitUtil.h>
#include <Common/Base/Memory/Allocator/Malloc/hkMallocAllocator.h>
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
#include <Physics/Collide/Util/Welding/hkpMeshWeldingUtility.h>
#include <Physics/Internal/Collide/Mopp/Code/hkpMoppCode.h>

#include <Physics/Collide/Shape/Compound/Collection/CompressedMesh/hkpCompressedMeshShapeBuilder.h>


#include <Common/Base/keycode.cxx>

#ifdef HK_FEATURE_PRODUCT_ANIMATION
#undef HK_FEATURE_PRODUCT_ANIMATION
#endif
#ifndef HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#define HK_EXCLUDE_LIBRARY_hkgpConvexDecomposition
#endif
#include <Common/Base/Config/hkProductFeatures.cxx> 

#pragma comment(lib, "hkBase.lib")
#pragma comment(lib, "hkSerialize.lib")
#pragma comment(lib, "hkpInternal.lib")
#pragma comment(lib, "hkpUtilities.lib")
#pragma comment(lib, "hkpCollide.lib")
#pragma comment(lib, "hkpConstraintSolver.lib")

/*-------------------------------------------------------------------------*/
static void HK_CALL errorReport(const char* msg, void*)
{
	std::cout << msg;
}

/*-------------------------------------------------------------------------*/
hkpMoppCode* buildCodeSimpleMesh(const hkpSimpleMeshShape *mesh, const hkpMoppCompilerInput *mfr)
{
	__try
	{
		return hkpMoppUtility::buildCode(mesh, *mfr);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

/*-------------------------------------------------------------------------*/
hkpMoppCode* buildCodeCompressedMesh(const hkpCompressedMeshShape* mesh, const hkpMoppCompilerInput *mfr)
{
	__try
	{
		return hkpMoppUtility::buildCode(mesh, *mfr);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return NULL;
	}
}

/*-------------------------------------------------------------------------*/
void mopperSimpleMesh(std::istream & infile)
{
	hkpMoppCode*							k_phkpMoppCode    (NULL);
	hkpSimpleMeshShape*						list              (new hkpSimpleMeshShape(0.01f));
	hkArray<hkVector4>&						vertices          (list->m_vertices);
	hkArray<hkpSimpleMeshShape::Triangle>&	triangles         (list->m_triangles);
	hkArray<hkUint8>&						materialindices   (list->m_materialIndices);
	hkpMoppCompilerInput					mfr;
	float									x                 (0.0);
	float									y                 (0.0);
	float									z                 (0.0);
	int										numvertices       (0);
	int										numtriangles;     (0);
	int										nummaterialindices(0);

	//  reset size of vectors
	vertices.setSize(0);
	triangles.setSize(0);
	materialindices.setSize(0);

	//  read number of vertices
	infile >> numvertices;

	//  read each vertex
	for (int i(0); i < numvertices; ++i)
	{
		//  read coordinates
		infile >> x >> y >> z;

		//  early break on eof or error
		if (infile.eof() || infile.fail())		return;

		//  add vertex to vector
		vertices.pushBack(hkVector4(x, y, z));
	}

	//  read number of triangles
	infile >> numtriangles;

	//  read each triangle
	for (int i(0); i < numtriangles; ++i)
	{
		hkpSimpleMeshShape::Triangle	hktri;

		//  read vertex indices
		infile >> hktri.m_a >> hktri.m_b >> hktri.m_c;

		//  early break on eof or error
		if (infile.eof() || infile.fail())		return;

		//  add triangle to vector
		triangles.pushBack(hktri);
	}

	//  read number of material indices
	infile >> nummaterialindices;

	//  read each material index
	for (int i(0); i < nummaterialindices; ++i)
	{
		hkUint8		matindex;

		//  read index
		infile >> matindex;

		//  early break on eof or error
		if (infile.eof() || infile.fail())		return;

		//  add material index to vector
		materialindices.pushBack(matindex);
	}

	//  create shape and compute welding
	//  initialize MoppInfo
	mfr.setAbsoluteFitToleranceOfAxisAlignedTriangles( hkVector4( 0.1945f, 0.1945f, 0.1945f ) );
	mfr.setAbsoluteFitToleranceOfTriangles(0.1945f);
	mfr.setAbsoluteFitToleranceOfInternalNodes(0.3f);

	//  build MoppCode
	k_phkpMoppCode = buildCodeSimpleMesh(list, &mfr);

	//  create pseudo shape
	hkpMoppBvTreeShape	bvtree(list, k_phkpMoppCode);

	//  compute welding info
	hkpMeshWeldingUtility::computeWeldingInfo(list, &bvtree, hkpWeldingUtility::WELDING_TYPE_ANTICLOCKWISE);

	//  return values in case of existing MoppCode
	if (k_phkpMoppCode != NULL)
	{
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
		// print welding info
		std::cout << triangles.getSize() << std::endl;
		for (int i = 0; i < triangles.getSize(); i++) {
			std::cout
				<< triangles[i].m_weldingInfo
				<< std::endl;
		}
	}  //  if (k_phkpMoppCode != NULL)

	// Deallocate shape
	if (list)
	{
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

/*-------------------------------------------------------------------------*/
void mopperCompressedMesh(std::istream & infile)
{
	hkpCompressedMeshShape*			pCompMesh    (NULL);
	hkpMoppCode*					pMoppCode    (NULL);
	hkpCompressedMeshShapeBuilder	shapeBuilder;
	hkpMoppCompilerInput			mci;
	float							x            (0);
	float							y            (0);
	float							z            (0);
	int								numGeometries(0);
	int								numVertices  (0);
	int								numTriangles (0);
	int								subPartId    (0);

	//  initialize shape Builder
	shapeBuilder.m_stripperPasses = 5000;

	//  create compressedMeshShape
	pCompMesh = shapeBuilder.createMeshShape(0.001f, hkpCompressedMeshShape::MATERIAL_SINGLE_VALUE_PER_CHUNK);

	//  read number of geometries
	infile >> numGeometries;

	//  read geometries
	for (int idxGeo(0); idxGeo < numGeometries; ++idxGeo)
	{
		hkGeometry						tGeo;
		hkArray<hkVector4>&				vertices (tGeo.m_vertices);
		hkArray<hkGeometry::Triangle>&	triangles(tGeo.m_triangles);

		//  reset sizes
		vertices.clear();
		triangles.clear();

		//  read number of vertices
		infile >> numVertices;

		//  read each vertex
		for (int i(0); i < numVertices; ++i)
		{
			//  read coordinates
			infile >> x >> y >> z;

			//  early break on eof or error
			if (infile.eof() || infile.fail())		return;

			//  add vertex to vector
			vertices.pushBack(hkVector4(x, y, z));
		}

		//  read number of triangles
		infile >> numTriangles;

		//  read each triangle
		for (int i(0); i < numTriangles; ++i)
		{
			hkGeometry::Triangle	triangle;

			//  read vertex indices
			infile >> triangle.m_a >> triangle.m_b >> triangle.m_c;

			//  early break on eof or error
			if (infile.eof() || infile.fail())		return;

			//  add vertex to vector
			triangles.pushBack(triangle);
		}

		//  material indices later on


		//  add geometry to CompressedMesh
		subPartId = shapeBuilder.beginSubpart(pCompMesh);
		shapeBuilder.addGeometry(tGeo, hkMatrix4::getIdentity(), pCompMesh);
		shapeBuilder.endSubpart (pCompMesh);
		shapeBuilder.addInstance(subPartId, hkMatrix4::getIdentity(), pCompMesh);

	}  //  for (int idxGeo(0); idxGeo < numGeometries; ++idxGeo)

	//  create shape and compute welding
	//  initialize MoppInfo
	mci.m_enableChunkSubdivision = true;

	//  build MoppCode
	pMoppCode = buildCodeCompressedMesh(pCompMesh, &mci);

	//  create pseudo shape
	hkpMoppBvTreeShape	bvtree(pCompMesh, pMoppCode);

	//  compute welding info
	hkpMeshWeldingUtility::computeWeldingInfo(pCompMesh, &bvtree, hkpWeldingUtility::WELDING_TYPE_TWO_SIDED);

	if ((pCompMesh != NULL) && (pMoppCode != NULL))
	{
		//  bhkMoppBvTree
		//  print mopp
		std::cout.precision(16);
		std::cout << pMoppCode->m_info.m_offset(0) << std::endl;
		std::cout << pMoppCode->m_info.m_offset(1) << std::endl;
		std::cout << pMoppCode->m_info.m_offset(2) << std::endl;
		std::cout << pMoppCode->m_info.getScale() << std::endl;
		std::cout << pMoppCode->m_data.getSize() << std::endl;

		//  print mopp code
		std::cout << int(pMoppCode->m_data[pMoppCode->m_data.getSize() - 1]) << std::endl;
		for (int i(0); i < (pMoppCode->m_data.getSize() - 1); ++i)
		{
			std::cout << int(pMoppCode->m_data[i]) << std::endl;
		}

		//  bhkCompressedMeshShapeData
		//  print boundings
		std::cout << pCompMesh->m_bounds.m_min(0) << std::endl;
		std::cout << pCompMesh->m_bounds.m_min(1) << std::endl;
		std::cout << pCompMesh->m_bounds.m_min(2) << std::endl;
		std::cout << pCompMesh->m_bounds.m_min(3) << std::endl;
		std::cout << pCompMesh->m_bounds.m_max(0) << std::endl;
		std::cout << pCompMesh->m_bounds.m_max(1) << std::endl;
		std::cout << pCompMesh->m_bounds.m_max(2) << std::endl;
		std::cout << pCompMesh->m_bounds.m_max(3) << std::endl;

		//  print bigVerts
		std::cout << pCompMesh->m_bigVertices.getSize() << std::endl;
		for (int i(0); i < pCompMesh->m_bigVertices.getSize(); ++i)
		{
			std::cout << pCompMesh->m_bigVertices[i](0) << std::endl;
			std::cout << pCompMesh->m_bigVertices[i](1) << std::endl;
			std::cout << pCompMesh->m_bigVertices[i](2) << std::endl;
			std::cout << pCompMesh->m_bigVertices[i](3) << std::endl;
		}

		//  print bigTriangles
		std::cout << pCompMesh->m_bigTriangles.getSize() << std::endl;
		for (int i(0); i < pCompMesh->m_bigTriangles.getSize(); ++i)
		{
			std::cout << pCompMesh->m_bigTriangles[i].m_a << std::endl;
			std::cout << pCompMesh->m_bigTriangles[i].m_b << std::endl;
			std::cout << pCompMesh->m_bigTriangles[i].m_c << std::endl;
			std::cout << pCompMesh->m_bigTriangles[i].m_material << std::endl;
			std::cout << pCompMesh->m_bigTriangles[i].m_weldingInfo << std::endl;
		}

		//  print transformations
		std::cout << pCompMesh->m_transforms.getSize() << std::endl;
		for (int i(0); i < pCompMesh->m_transforms.getSize(); ++i)
		{
			std::cout << pCompMesh->m_transforms[i].m_translation(0) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_translation(1) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_translation(2) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_translation(3) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_rotation(0) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_rotation(1) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_rotation(2) << std::endl;
			std::cout << pCompMesh->m_transforms[i].m_rotation(3) << std::endl;
		}

		//  print chunks
		std::cout << pCompMesh->m_chunks.getSize() << std::endl;

		//  for each chunk
		for (int i(0); i < pCompMesh->m_chunks.getSize(); ++i)
		{
			//  print translation
			std::cout << pCompMesh->m_chunks[i].m_offset(0) << std::endl;
			std::cout << pCompMesh->m_chunks[i].m_offset(1) << std::endl;
			std::cout << pCompMesh->m_chunks[i].m_offset(2) << std::endl;
			std::cout << pCompMesh->m_chunks[i].m_offset(3) << std::endl;

			//  print forced flags
			std::cout << pCompMesh->m_chunks[i].m_materialInfo << std::endl;
			std::cout << 65535 << std::endl;
			std::cout << pCompMesh->m_chunks[i].m_transformIndex << std::endl;

			//  print vertices
			std::cout << pCompMesh->m_chunks[i].m_vertices.getSize() << std::endl;
			for (int t(0); t < pCompMesh->m_chunks[i].m_vertices.getSize(); ++t)
			{
				std::cout << pCompMesh->m_chunks[i].m_vertices[t] << std::endl;
			}

			//  print indices
			std::cout << pCompMesh->m_chunks[i].m_indices.getSize() << std::endl;
			for (int t(0); t < pCompMesh->m_chunks[i].m_indices.getSize(); ++t)
			{
				std::cout << pCompMesh->m_chunks[i].m_indices[t] << std::endl;
			}

			//  print strips
			std::cout << pCompMesh->m_chunks[i].m_stripLengths.getSize() << std::endl;
			for (int t(0); t < pCompMesh->m_chunks[i].m_stripLengths.getSize(); ++t)
			{
				std::cout << pCompMesh->m_chunks[i].m_stripLengths[t] << std::endl;
			}

			//  print welding info
			std::cout << pCompMesh->m_chunks[i].m_weldingInfo.getSize() << std::endl;
			for (int t(0); t < pCompMesh->m_chunks[i].m_weldingInfo.getSize(); ++t)
			{
				std::cout << pCompMesh->m_chunks[i].m_weldingInfo[t] << std::endl;
			}
		}  //  for (int i(0); i < pCompMesh->m_chunks.getSize(); ++i)
	}  //  if ((pCompMesh != NULL) && (pMoppCode != NULL))

	// Deallocate shape
	if (pCompMesh)
	{
		pCompMesh->removeReference();
		pCompMesh = NULL;
	}

	// Deallocate mopp code
	if (pMoppCode)
	{
		pMoppCode->removeReference();
		pMoppCode = NULL;
	}
}

/*-------------------------------------------------------------------------*/
//int _tmain(int argc, _TCHAR* argv[])
int main(int argc, char *argv[])
{
	if (argc < 2) {
		std::cout
			<< "Mopper. Copyright (c) 2008-2012, NIF File Format Library and Tools." << std::endl
			<< "All rights reserved." << std::endl
			<< std::endl
			<< "Options:" << std::endl
			<< "  --help      for usage help" << std::endl
			<< "  --license   for licensing details" << std::endl
			<< std::endl
			<< "Mopper uses havok. "
			<< "Copyright 1999-2012 Havok.com Inc. (and its Licensors)."
			<< std::endl
			<< "All Rights Reserved. See www.havok.com for details."
			<< std::endl << std::endl;
		return 0;
	}

	if (std::strcmp(argv[1], "--help") == 0) {
		std::cout <<
"usage: mopper.exe [command] [<file>|--]\n\n"
"command:\n"
"  -msm\t: create MOPP data for bhkSimpleMeshShape\n"
"  -ccm\t: create complete bhkCompressedMeshShape\n\n"
"where <file> (-- for standard input) is of the following format\n for bhkSimpleMeshShape:\n"
"<number of vertices>\n"
"<vertex 1 x> <vertex 1 y> <vertex 1 z>\n"
"<vertex 2 x> <vertex 2 y> <vertex 2 z>\n"
"...\n"
"<number of triangles>\n"
"<triangle 1 index 0> <triangle 1 index 1> <triangle 1 index 2>\n"
"<triangle 2 index 0> <triangle 2 index 1> <triangle 2 index 2>\n"
"...\n\n"
"where <file> (-- for standard input) is of the following format\n for bhkComplexMeshShape:\n"
"<number of geometries>\n"
"<number of vertices geometry 1>\n"
"<vertex 1 x> <vertex 1 y> <vertex 1 z>\n"
"<vertex 2 x> <vertex 2 y> <vertex 2 z>\n"
"...\n"
"<number of triangles geometry 1>\n"
"<triangle 1 index 0> <triangle 1 index 1> <triangle 1 index 2>\n"
"<triangle 2 index 0> <triangle 2 index 1> <triangle 2 index 2>\n"
"...\n"
"<number of vertices geometry 2>\n"
"<vertex 1 x> <vertex 1 y> <vertex 1 z>\n"
"<vertex 2 x> <vertex 2 y> <vertex 2 z>\n"
"...\n"
"<number of triangles geometry 2>\n"
"<triangle 1 index 0> <triangle 1 index 1> <triangle 1 index 2>\n"
"<triangle 2 index 0> <triangle 2 index 1> <triangle 2 index 2>\n"
"...\n"
"<number of vertices geometry n>\n"
"<vertex 1 x> <vertex 1 y> <vertex 1 z>\n"
"<vertex 2 x> <vertex 2 y> <vertex 2 z>\n"
"...\n"
"<number of triangles geometry n>\n"
"<triangle 1 index 0> <triangle 1 index 1> <triangle 1 index 2>\n"
"<triangle 2 index 0> <triangle 2 index 1> <triangle 2 index 2>\n"
"...\n\n";
		return 0;
	}

	if (std::strcmp(argv[1], "--license") == 0) {
		std::cout <<
"Mopper. Copyright (c) 2008-2012, NIF File Format Library and Tools\n"
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

	// Initialize the base system including our memory system
	hkMemoryRouter*		pMemoryRouter(hkMemoryInitUtil::initDefault(hkMallocAllocator::m_defaultMallocAllocator, hkMemorySystem::FrameInfo(500000)));
	hkBaseSystem::init(pMemoryRouter, errorReport);

	// Call main program.
	if (std::strcmp(argv[1], "--") == 0)  //  backward compatibility
	{
		mopperSimpleMesh(std::cin);
	}
	else if (std::strcmp(argv[1], "-msm") == 0)
	{
		if (std::strcmp(argv[2], "--") == 0)
		{
			mopperSimpleMesh(std::cin);
		}
		else
		{
			mopperSimpleMesh(std::ifstream(argv[2], std::ifstream::in));
		}
	}
	else if (std::strcmp(argv[1], "-ccm") == 0)
	{
		if (std::strcmp(argv[2], "--") == 0)
		{
			mopperCompressedMesh(std::cin);
		}
		else
		{
			mopperCompressedMesh(std::ifstream(argv[2], std::ifstream::in));
		}
	}
	else  //  backward compatibility
	{
		mopperSimpleMesh(std::ifstream(argv[1], std::ifstream::in));
	}

	// Quit base system
	hkBaseSystem::quit();

	return 0;
}
