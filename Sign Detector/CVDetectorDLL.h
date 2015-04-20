#ifdef SIGNDETECTORSDLL_EXPORTS
#define SIGNDETECTORSDLL_API __declspec(dllexport) 
#else
#define SIGNDETECTORSDLL_API __declspec(dllimport) 
#endif

namespace CVDetector
{
    class SignDetector
    {
    public: 
        // Returns -1 if failed, 0 if OK
        static SIGNDETECTORSDLL_API int initialiseDetector(); 

        static SIGNDETECTORSDLL_API void detectSigns(); 

        // Returns sign ID found by the detector (0, 1 or 2)
        static SIGNDETECTORSDLL_API int getFoundSign(); 
    };

	class RoadTracker
	{
		public: 
        // Returns -1 if failed, 0 if OK
        static SIGNDETECTORSDLL_API int initialiseHSVTracker(); 

        static SIGNDETECTORSDLL_API void runTracker(); 

        // Returns Y coordinate of the center of the detected patch
        static SIGNDETECTORSDLL_API int getAxis(); 
	};
}