#include <openvdb/openvdb.h>
#include <openvdb/tools/LevelSetSphere.h>
#include <tbb/parallel_for.h>
#include <tbb/atomic.h>
#include <cassert>
#include <iostream>

#include "ofMain.h"
#include "ofApp.h"

// Global active voxel counter, atomically updated from multiple threads
tbb::atomic<openvdb::Index64> activeLeafVoxelCount;
// Functor for use with tbb::parallel_for() that operates on a grid's leaf nodes
template<typename GridType>
struct LeafProcessor
{
    typedef typename GridType::TreeType TreeType;
    typedef typename TreeType::LeafNodeType LeafNode;
    // Define an IteratorRange that splits the iteration space of a leaf iterator.
    typedef openvdb::tree::IteratorRange<typename TreeType::LeafCIter> IterRange;
    void operator()(IterRange& range) const
    {
        // Note: this code must be thread-safe.
        // Iterate over a subrange of the leaf iterator's iteration space.
        for ( ; range; ++range)
        {
            // Retrieve the leaf node to which the iterator is pointing.
            const LeafNode& leaf = *range.iterator();
            // Update the global counter.
            activeLeafVoxelCount.fetch_and_add(leaf.onVoxelCount());
        }
    }
};


void voxels(){
  openvdb::initialize();
    // Generate a level set grid.
    openvdb::FloatGrid::Ptr grid =
        openvdb::tools::createLevelSetSphere<openvdb::FloatGrid>(/*radius=*/20.0,
                /*center=*/openvdb::Vec3f(1.5, 2, 3), /*voxel size=*/0.5);
    // Construct a functor for use with tbb::parallel_for()
    // that processes the leaf nodes of a FloatGrid.
    typedef LeafProcessor<openvdb::FloatGrid> FloatLeafProc;
    FloatLeafProc proc;
    // Wrap a leaf iterator in an IteratorRange.
    FloatLeafProc::IterRange range(grid->tree().cbeginLeaf());
    // Iterate over leaf nodes in parallel.
    tbb::parallel_for(range, proc);
    std::cout << activeLeafVoxelCount << " active leaf voxels" << std::endl;
    // The computed voxel count should equal the grid's active voxel count,
    // since all of the active voxels in a level set grid are stored at the
    // leaf level (that is, there are no active tiles in a level set grid).
    assert(activeLeafVoxelCount == grid->activeVoxelCount());
}

//========================================================================
int main( )
{



    ofSetupOpenGL(1024,768, OF_WINDOW);			// <-------- setup the GL context

    // this kicks off the running of my app
    // can be OF_WINDOW or OF_FULLSCREEN
    // pass in width and height too:
    ofRunApp( new ofApp());

}
