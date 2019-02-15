#include <iostream>
#include <fstream>
#include <unistd.h>

#include <pcl/point_cloud.h>
#include <pcl/io/ply_io.h>

#include <cwipc_codec/api.h>

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << "pointcloudfile.ply compressedfile.cwicpc" << std::endl;
        return 2;
    }
    //
    // Read pointcloud file
    //
    pcl::PointCloud<pcl::PointXYZRGB> *pc = new pcl::PointCloud<pcl::PointXYZRGB>;
    pcl::PLYReader ply_reader;
#ifdef WITH_PLY_READER_BUG
	pcl::PolygonMesh mesh;
    if (ply_reader.read(argv[1], mesh) < 0) {
        std::cerr << argv[0] << ": Error reading mesh from " << argv[1] << std::endl;
        return 1;
    }
    pcl::fromPCLPointCloud2(mesh.cloud, *pc);
#else
    if (ply_reader.read(argv[1], *pc) < 0) {
        std::cerr << argv[0] << ": Error reading pointcloud from " << argv[1] << std::endl;
        return 1;
    }

#endif
    std::cerr << "Read pointcloud successfully, " << pc->size() << " points." << std::endl;
    //
    // Compress
    //
    encoder_params param;
	param.num_threads = 1;
	param.do_inter_frame = false;
	param.gop_size = 1;
	param.exp_factor = 0;
	param.octree_bits = 7;
	param.color_bits = 8;
	param.jpeg_quality = 85;
	param.macroblock_size = 16;
    cwi_encode encoder;
    std::stringstream outputBuffer;
    void *pcVoidPtr = reinterpret_cast<void*>(pc);
    if (encoder.cwi_encoder(param, pcVoidPtr, outputBuffer, 0) < 0) {
        std::cerr << argv[0] << ": Error encoding pointcloud from " << argv[1] << std::endl;
        return 1;
    }
    std::cerr << "Encoded successfully, " << outputBuffer.tellp() << " bytes." << std::endl;
    //
    // Save to output
    //
    std::ofstream output(argv[2]);
    output << outputBuffer.rdbuf();
    output.close();
    return 0;
}
