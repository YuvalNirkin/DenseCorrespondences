#ifndef __DENSE_SIFT_EXPORT__
#define __DENSE_SIFT_EXPORT__

#if (defined WIN32 || defined _WIN32 || defined WINCE) && defined DENSESIFT_EXPORTS
#  define DENSE_SIFT_API __declspec(dllexport)
#else
#  define DENSE_SIFT_API
#endif

#endif	// __DENSE_SIFT_EXPORT__