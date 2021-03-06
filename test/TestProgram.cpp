/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#include "TestProgram.h"

#include "src/Program.h"
#include "src/icd_loader.h"
#include "util.h"

using namespace vc4cl;

uint32_t hello_world_vector_hex[] = {
    #include "hello_world_vector.hex"
};

static std::string sourceCode;

TestProgram::TestProgram() : num_callback(0), context(NULL), source_program(NULL), binary_program(NULL)
{
    TEST_ADD(TestProgram::testCreateProgramWithSource);
    TEST_ADD(TestProgram::testCreateProgramWithBinary);
    TEST_ADD(TestProgram::testCreateProgramWithBuiltinKernels);
    TEST_ADD(TestProgram::testBuildProgram);
    TEST_ADD(TestProgram::testCompileProgram);
    TEST_ADD(TestProgram::testLinkProgram);
    TEST_ADD(TestProgram::testUnloadPlatformCompiler);
    TEST_ADD(TestProgram::testGetProgramInfo);
    TEST_ADD(TestProgram::testGetProgramBuildInfo);
    TEST_ADD(TestProgram::testRetainProgram);
    TEST_ADD(TestProgram::testReleaseProgram);
}

bool TestProgram::setup()
{
    cl_int errcode = CL_SUCCESS;
    cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    context = VC4CL_FUNC(clCreateContext)(NULL, 1, &device_id, NULL, NULL, &errcode);
    return errcode == CL_SUCCESS && context != NULL;
}

void TestProgram::testCreateProgramWithSource()
{
    cl_int errcode = CL_SUCCESS;
    sourceCode = readFile("./test/hello_world_vector.cl");
    const std::size_t sourceLength = sourceCode.size();
    source_program = VC4CL_FUNC(clCreateProgramWithSource)(context, 1, NULL, &sourceLength, &errcode);
    TEST_ASSERT(errcode != CL_SUCCESS);
    TEST_ASSERT_EQUALS(nullptr, source_program);
    
    const char* strings[1] = {sourceCode.data()};
    source_program = VC4CL_FUNC(clCreateProgramWithSource)(context, 1, strings, &sourceLength, &errcode);
    TEST_ASSERT_EQUALS(CL_SUCCESS, errcode);
    TEST_ASSERT(source_program != NULL);
}

void TestProgram::testCreateProgramWithBinary()
{
    cl_int errcode = CL_SUCCESS;
    cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    binary_program = VC4CL_FUNC(clCreateProgramWithBinary)(context, 1, &device_id, NULL, NULL, NULL, &errcode);
    TEST_ASSERT(errcode != CL_SUCCESS);
    TEST_ASSERT_EQUALS(nullptr, binary_program);
    
    cl_int binary_state = CL_SUCCESS;
    size_t binary_size = sizeof(hello_world_vector_hex);
    const unsigned char* strings[1] = {(const unsigned char*)hello_world_vector_hex};
    binary_program = VC4CL_FUNC(clCreateProgramWithBinary)(context, 1, &device_id, &binary_size, strings, &binary_state, &errcode);
    TEST_ASSERT_EQUALS(CL_SUCCESS, errcode);
    TEST_ASSERT(binary_program != NULL);
    TEST_ASSERT_EQUALS(CL_SUCCESS, binary_state);
}

void TestProgram::testCreateProgramWithBuiltinKernels()
{
    cl_int errcode = CL_SUCCESS;
    cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    cl_program program = VC4CL_FUNC(clCreateProgramWithBuiltInKernels)(context, 1, &device_id, "", &errcode);
    TEST_ASSERT(errcode != CL_SUCCESS);
    TEST_ASSERT_EQUALS(nullptr, program);
}

static void build_callback(cl_program prog, void* test)
{
    ((TestProgram*)test)->num_callback += 1;
}

void TestProgram::testBuildProgram()
{
	cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    cl_int state = VC4CL_FUNC(clBuildProgram)(binary_program, 1, &device_id, NULL, &build_callback, this);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
}

void TestProgram::testCompileProgram()
{
	cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    cl_int state = VC4CL_FUNC(clCompileProgram)(source_program, 1, &device_id, "-Wall", 0, NULL, NULL, &build_callback, this);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
}

void TestProgram::testLinkProgram()
{
    cl_int errcode = CL_SUCCESS;
    cl_device_id device_id = Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase();
    cl_program program = VC4CL_FUNC(clLinkProgram)(context, 1, &device_id, NULL, 1, &source_program, &build_callback, this, &errcode);
    TEST_ASSERT_EQUALS(CL_SUCCESS, errcode);
    TEST_ASSERT_EQUALS(source_program, program);
}

void TestProgram::testUnloadPlatformCompiler()
{
    cl_int state = VC4CL_FUNC(clUnloadPlatformCompiler)(NULL);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    
    TEST_ASSERT_EQUALS(4, num_callback);
}

void TestProgram::testGetProgramBuildInfo()
{
    size_t info_size = 0;
    char buffer[2048];
    cl_int state = VC4CL_FUNC(clGetProgramBuildInfo)(source_program, Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase(), CL_PROGRAM_BUILD_STATUS, 2048, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_build_status), info_size);
    TEST_ASSERT_EQUALS(CL_BUILD_SUCCESS, *(cl_build_status*)buffer);
    
    state = VC4CL_FUNC(clGetProgramBuildInfo)(source_program, Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase(), CL_PROGRAM_BUILD_OPTIONS, 2048, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT(info_size > 0);
    
    state = VC4CL_FUNC(clGetProgramBuildInfo)(source_program, Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase(), CL_PROGRAM_BUILD_LOG, 2048, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT(info_size > 0);
    
    state = VC4CL_FUNC(clGetProgramBuildInfo)(source_program, Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase(), CL_PROGRAM_BINARY_TYPE, 2048, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_program_binary_type), info_size);
    TEST_ASSERT_EQUALS(CL_PROGRAM_BINARY_TYPE_EXECUTABLE, *(cl_program_binary_type*)buffer);
}

void TestProgram::testGetProgramInfo()
{
    size_t info_size = 0;
    char buffer[2048];
    cl_int state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_REFERENCE_COUNT, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_uint), info_size);
    TEST_ASSERT_EQUALS(1, *(cl_uint*)buffer);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_CONTEXT, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_context), info_size);
    TEST_ASSERT_EQUALS(context, *(cl_context*)buffer);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_NUM_DEVICES, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_uint), info_size);
    TEST_ASSERT_EQUALS(1, *(cl_uint*)buffer);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_DEVICES, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(cl_device_id), info_size);
    TEST_ASSERT_EQUALS(Platform::getVC4CLPlatform().VideoCoreIVGPU.toBase(), *(cl_device_id*)buffer);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_SOURCE, 128, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_INVALID_VALUE, state);   //buffer-size!
    TEST_ASSERT(info_size > 0);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_SOURCE, 2048, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT(info_size > 0 && info_size < 2048);
    
    state = VC4CL_FUNC(clGetProgramInfo)(binary_program, CL_PROGRAM_SOURCE, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(1, info_size);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_BINARY_SIZES, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(size_t), info_size);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_BINARIES, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_NUM_KERNELS, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(sizeof(size_t), info_size);
    TEST_ASSERT_EQUALS(1, *(size_t*)buffer);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, CL_PROGRAM_KERNEL_NAMES, 1024, buffer, &info_size);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    
    state = VC4CL_FUNC(clGetProgramInfo)(source_program, 0xDEADBEAF, 1024, buffer, &info_size);
    TEST_ASSERT(state != CL_SUCCESS);
}

void TestProgram::testRetainProgram()
{
    TEST_ASSERT_EQUALS(1, toType<Program>(source_program)->getReferences());
    cl_int state = VC4CL_FUNC(clRetainProgram)(source_program);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    
    TEST_ASSERT_EQUALS(2, toType<Program>(source_program)->getReferences());
    state = VC4CL_FUNC(clReleaseProgram)(source_program);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    TEST_ASSERT_EQUALS(1, toType<Program>(source_program)->getReferences());
}

void TestProgram::testReleaseProgram()
{
    TEST_ASSERT_EQUALS(1, toType<Program>(source_program)->getReferences());
    cl_int state = VC4CL_FUNC(clReleaseProgram)(source_program);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
    
    TEST_ASSERT_EQUALS(1, toType<Program>(binary_program)->getReferences());
    state = VC4CL_FUNC(clReleaseProgram)(binary_program);
    TEST_ASSERT_EQUALS(CL_SUCCESS, state);
}

void TestProgram::tear_down()
{
    VC4CL_FUNC(clReleaseContext)(context);
}
