/*
 * Author: doe300
 *
 * See the file "LICENSE" for the full license governing this code.
 */

#ifndef VC4CL_PLATFORM
#define VC4CL_PLATFORM

#include "Object.h"
#include "Device.h"

namespace vc4cl
{
	class Platform : public Object<_cl_platform_id, CL_INVALID_PLATFORM>
	{
	public:
		~Platform();
		CHECK_RETURN cl_int getInfo(cl_platform_info param_name, size_t param_value_size, void* param_value, size_t* param_value_size_ret) const;

		static Platform& getVC4CLPlatform();

		Device VideoCoreIVGPU;
	private:
		Platform();
		Platform(const Platform&) = delete;
		Platform(Platform&&) = delete;
	};


} /* namespace vc4c */

#endif /* VC4CL_PLATFORM */
