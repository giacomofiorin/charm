/**
 * A tetrahedral mesh.
 * 
 * Orion Sky Lawlor, olawlor@acm.org, 3/12/2003
 */
#ifndef __UIUC_CHARM_TETMESH_H
#define __UIUC_CHARM_TETMESH_H

#include <stdio.h> // For FILE *
#include "ckvector3d.h"
#include <vector>

#define OSL_TETMESH_DEBUG 0

/**
 * A 3d tetrahedral mesh.  Contains the connectivity only--no data.
 */
class TetMesh {
public:
	enum {nodePer=4}; ///< Nodes per tet.
	
	// Connectivity: 0-based node indices around our tet.
	class conn_t {
	public:
		int nodes[TetMesh::nodePer];
		conn_t() {nodes[0]=-1;}
		conn_t(int a,int b,int c,int d)
			{nodes[0]=a; nodes[1]=b; nodes[2]=c; nodes[3]=d;}
	};
	
	/// Create a new empty mesh.
	TetMesh();
	/// Create a new mesh with this many tets and points.
	TetMesh(int nt,int np);
	virtual ~TetMesh();
	
	/// Set the size of this mesh to be nt tets and np points.
	///  Throws away the previous mesh.
	virtual void allocate(int nt,int np);
	
	/// Return the number of tets in the mesh
	inline int getTets(void) const {return tet.size();}
	/// Return the t'th tetrahedra's 0-based node indices
	inline int *getTet(int t) {ct(t); return &(tet[t].nodes[0]);}
	inline const int *getTet(int t) const {ct(t); return &(tet[t].nodes[0]);}
	inline int *getTetConn(void) {return getTet(0);}
	inline const int *getTetConn(void) const {return getTet(0);}
	double getTetVolume(int t) const;
	
	/// Return the number of points (vertices, nodes) in the mesh
	inline int getPoints(void) const {return pts.size();}
	/// Return the p'th vertex (0..getPoints()-1)
	inline CkVector3d &getPoint(int p) {cp(p); return pts[p];}
	inline const CkVector3d &getPoint(int p) const {cp(p); return pts[p];}
	CkVector3d *getPointArray(void);
	const CkVector3d *getPointArray(void) const;
	
	/// Simple mesh modification.  
	///  The new number of the added object is returned.
	int addTet(const conn_t &c) {tet.push_back(c); return tet.size()-1;}
	int addPoint(const CkVector3d &pt) {pts.push_back(pt); return pts.size()-1;}

private:
	std::vector<conn_t> tet; //< Connectivity
	std::vector<CkVector3d> pts; //< nPts 3d node locations.
	
	///Check these indices for in-range
#if OSL_TETMESH_DEBUG /* Slow bounds checks */
	void ct(int t) const;
	void cp(int p) const;
#else /* Fast unchecked version for production code */
	inline void ct(int t) const {}
	inline void cp(int p) const {}
#endif
};

/// Print a debugging representation of this mesh, to stdout.
void print(const TetMesh &t);

/// Print a debugging representation of this mesh's size, to stdout.
void printSize(const TetMesh &t);

/// Print a debugging representation of this tet to stdout
void printTet(const TetMesh &m,int t);

/// Print a debugging point
void print(const CkVector3d &p);

/// Read a TetMesh (ghs3d) ".noboite" mesh description file.
/// Aborts on errors.
void readNoboite(FILE *f,TetMesh &t);
void writeNoboite(FILE *f,TetMesh &t);

/// Read this mesh from the FEM framework's mesh m
void readFEM(int m,TetMesh &t);

/// Write this mesh to the FEM framework's mesh m
void writeFEM(int m,TetMesh &t);

namespace cg3d { class Planar3dDest; };

/**
 * Return the average edge length on this mesh.
 */
double averageEdgeLength(const TetMesh &m);

#endif
