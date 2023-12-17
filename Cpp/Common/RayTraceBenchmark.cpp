#ifdef WIN32
#include "../RayTraceBenchmark/RayTraceBenchmark/stdafx.h"
#include <Windows.h>
#endif

#if !defined(WIN32)
#include <unistd.h>
#endif

#include <math.h>
#include <float.h>
#include <ctime>
#include <fstream>
#include <iostream>
using namespace std;

#ifdef IOS
#include "RayTraceBenchmark.cpp"
#endif

//#define BIT64
#ifdef BIT64
#define Num double
#define SQRT sqrt
#define POW pow
#define TAN tan
#define MAX_FT DBL_MAX
#else
#define Num float
#define SQRT sqrtf
#define POW powf
#define TAN tanf
#define MAX_FT FLT_MAX
#endif

#define INLINE inline

namespace RayTraceBenchmark
{
	// ==============================================
	// Main Benchmark Code
	// ==============================================
	struct Vec3
	{
	public:
		Num X, Y, Z;

		Vec3()
		{
			X = 0;
			Y = 0;
			Z = 0;
		}

		Vec3(Num x, Num y, Num z)
		{
			X = x;
			Y = y;
			Z = z;
		}

		INLINE
		Vec3 operator+(const Vec3& p)
		{
			return Vec3(X + p.X, Y + p.Y, Z + p.Z);
		}

		INLINE
		void operator+=(const Vec3& p)
		{
			this->X += p.X;
			this->Y += p.Y;
			this->Z += p.Z;
		}

		INLINE
		Vec3 operator-(const Vec3& p)
		{
			return Vec3(X - p.X, Y - p.Y, Z - p.Z);
		}

		INLINE
		Vec3 operator-()
		{
			return Vec3(-X, -Y, -Z);
		}

		INLINE
		Vec3 operator*(const Vec3& p)
		{
			return Vec3(X * p.X, Y * p.Y, Z * p.Z);
		}

		INLINE
		Vec3 operator*(const float p)
		{
			return Vec3(X * p, Y * p, Z * p);
		}

		INLINE
		Vec3 operator/(const Vec3& p)
		{
			return Vec3(X / p.X, Y / p.Y, Z / p.Z);
		}

		INLINE
		Vec3 operator/(const float p)
		{
			return Vec3(X / p, Y / p, Z / p);
		}

		INLINE
		static Num Dot(Vec3 v1, Vec3 v2)
		{
			return (v1.X*v2.X) + (v1.Y*v2.Y) + (v1.Z*v2.Z);
		}

		INLINE
		static Num Magnitude(Vec3 v)
		{
			return SQRT((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z));
		}

		INLINE
		static Vec3 Normalize(Vec3 v)
		{
			return v / SQRT((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z));
		}
	};

	struct Ray
	{
	public:
		Vec3 Org;
		Vec3 Dir;
	};

	class Sphere
	{
	public:
		Vec3 Center;
		Num Radius;
		Vec3 Color;
		Num Reflection;
		Num Transparency;

		Sphere(Vec3 c, Num r, Vec3 clr, Num refl = 0, Num trans = 0)
		{
			Center = c;
			Radius = r;
			Color = clr;
			Reflection = refl;
			Transparency = trans;
		}

		static Vec3 Normal(Sphere& sphere, Vec3 pos)
		{
			return Vec3::Normalize(pos - sphere.Center);
		}

		static bool Intersect(Sphere& sphere, Ray ray)
		{
			auto l = sphere.Center - ray.Org;
			auto a = Vec3::Dot(l, ray.Dir);
			if (a < 0)              // opposite direction
				return false;

			auto b2 = Vec3::Dot(l, l) - (a * a);
			auto r2 = sphere.Radius * sphere.Radius;
			if (b2 > r2)            // perpendicular > r
				return false;

			return true;
		}

		static bool Intersect(Sphere& sphere, Ray ray, Num* distance)
		{
			*distance = 0;

			auto l = sphere.Center - ray.Org;
			auto a = Vec3::Dot(l, ray.Dir);
			if (a < 0)              // opposite direction
				return false;

			auto b2 = Vec3::Dot(l, l) - (a * a);
			auto r2 = sphere.Radius * sphere.Radius;
			if (b2 > r2)            // perpendicular > r
				return false;

			Num c = SQRT(r2 - b2);
			Num _near = a - c;
			Num _far  = a + c;
			*distance = (_near < 0) ? _far : _near;
			// near < 0 means ray starts inside
			return true;
		}
	};

	class Light
	{
	public:
		Vec3 Position;
		Vec3 Color;

		Light(Vec3 position, Vec3 color)
		{
			Position = position;
			Color = color;
		}
	};

	class Scene
	{
	public:
		int ObjectCount, LightCount;
		Sphere** Objects;
		Light** Lights;
	};

	class Benchmark
	{
	public:
		#define Width (1280 * 16)
		#define Height (720 * 16)
		#ifdef BIT64
		#define fov 45.0
		#define PI 3.1415926535897932384626433832795
		#else
		#define fov 45.0f
		#define PI 3.1415926535897932384626433832795f
		#endif
		#define maxDepth 6

		static Vec3 trace (Ray ray, const Scene& scene, int depth)
		{
			Num nearest = MAX_FT;
			Sphere* obj = 0;

			// search the scene for nearest intersection
			for (int i = 0; i != scene.ObjectCount; ++i)
			{
				Num distance = MAX_FT;
				auto o = scene.Objects[i];
				if (Sphere::Intersect(*o, ray, &distance))
				{
					if (distance < nearest)
					{
						nearest = distance;
						obj = o;
					}
				}
			}

			if (obj == 0) return Vec3();

			auto point_of_hit = ray.Org + (ray.Dir * nearest);
			auto normal = Sphere::Normal(*obj, point_of_hit);
			bool inside = false;

			if (Vec3::Dot(normal, ray.Dir) > 0)
			{
				inside = true;
				normal = -normal;
			}

			Vec3 color = Vec3();
			auto reflection_ratio = obj->Reflection;

			for (int i = 0; i != scene.LightCount; ++i)
			{
				auto l = scene.Lights[i];
				auto light_direction = Vec3::Normalize(l->Position - point_of_hit);
				Ray r;
				#ifdef BIT64
				r.Org = point_of_hit + (normal * 1e-5);
				#else
				r.Org = point_of_hit + (normal * 1e-5f);
				#endif
				r.Dir = light_direction;

				// go through the scene check whether we're blocked from the lights
				bool blocked = false;
				for (int i = 0; i != scene.ObjectCount; ++i)
				{
					auto o = scene.Objects[i];
					if (Sphere::Intersect(*o, r))
					{
						blocked = true;
						break;
					}
				}

				if (!blocked)
				{
					color += l->Color * max(0.0f, Vec3::Dot(normal, light_direction)) * obj->Color * (1 - reflection_ratio);
				}
			}

			auto rayNormDot = Vec3::Dot(ray.Dir, normal);
			Num facing = max(0.0f, -rayNormDot);
			Num fresneleffect = reflection_ratio + ((1 - reflection_ratio) * POW((1 - facing), 5));

			// compute reflection
			if (depth < maxDepth && reflection_ratio > 0)
			{
				auto reflection_direction = ray.Dir + (normal * 2 * rayNormDot * (-1));
				Ray r;
				#ifdef BIT64
				r.Org = point_of_hit + (normal * 1e-5);
				#else
				r.Org = point_of_hit + (normal * 1e-5f);
				#endif
				r.Dir = reflection_direction;
				auto reflection = trace(r, scene, depth + 1);
				color += reflection * fresneleffect;
			}

			// compute refraction
			if (depth < maxDepth && (obj->Transparency > 0))
			{
				#ifdef BIT64
				Num ior = 1.5;
				#else
				Num ior = 1.5f;
				#endif
				auto CE = Vec3::Dot(ray.Dir, normal) * (-1);
				ior = inside ? (1) / ior : ior;
				Num eta = 1 / ior;
				auto GF = (ray.Dir + normal * CE) * eta;
				Num sin_t1_2 = 1 - (CE * CE);
				Num sin_t2_2 = sin_t1_2 * (eta * eta);
				if (sin_t2_2 < 1)
				{
					auto GC = normal * SQRT(1 - sin_t2_2);
					auto refraction_direction = GF - GC;
					Ray r;
					#ifdef BIT64
					r.Org = point_of_hit - (normal * 1e-4);
					#else
					r.Org = point_of_hit - (normal * 1e-4f);
					#endif
					r.Dir = refraction_direction;
					auto refraction = trace(r, scene, depth + 1);
					color += refraction * (1 - fresneleffect) * obj->Transparency;
				}
			}
			return color;
		}

		static char* Render(Scene& scene, char* pixels)
		{
			auto eye = Vec3();
			Num h = TAN(((fov / 360) * (2 * PI)) / 2) * 2;
			Num w = (h * Width) / Height;

			for (int y = 0; y != Height; ++y)
			{
				for (int x = 0; x != Width; ++x)
				{
					Num xx = x, yy = y, ww = Width, hh = Height;
					Vec3 dir;
					dir.X = ((xx - (ww / 2)) / ww) * w;
					dir.Y = (((hh / 2) - yy) / hh) * h;
					dir.Z = -1;
					dir = Vec3::Normalize(dir);

					Ray r;
					r.Org = eye;
					r.Dir = dir;
					auto pixel = trace(r, scene, 0);
					int i = (x * 3) + (y * Width * 3);
					pixels[i] = min(pixel.X * 255.0f, 255.0f);
					pixels[i+1] = min(pixel.Y * 255.0f, 255.0f);
					pixels[i+2] = min(pixel.Z * 255.0f, 255.0f);
				}
			}

			return pixels;
		}
	};

	// ==============================================
	// Prep Code
	// ==============================================
	#ifdef WIN32
	TIMECAPS caps;
	#endif

	class BenchmarkMain
	{
	public:
		#ifdef WIN32
		static void Win32OptimizedStopwatch()
		{
			caps = TIMECAPS();
			if (timeGetDevCaps(&caps, sizeof(TIMECAPS)) != 0)
			{
				cout << "StopWatch: TimeGetDevCaps failed";
			}
			
			if (timeBeginPeriod(caps.wPeriodMin) != 0)
			{
				cout << "StopWatch: TimeBeginPeriod failed";
			}
		}

		static void Win32EndOptimizedStopwatch()
		{
			if (timeEndPeriod(caps.wPeriodMin) != 0)
			{
				cout << "StopWatch: TimeEndPeriod failed";
			}
		}
		#endif

		static double Start()
		{
			// create objects
			auto scene = new Scene();
			scene->ObjectCount = 5;
			scene->Objects = new Sphere*[5]
			{
				new Sphere(Vec3(0.0f, -10002.0f, -20.0f), 10000, Vec3(.8f, .8f, .8f)),
				new Sphere(Vec3(0.0f, 2.0f, -20.0f), 4, Vec3(.8f, .5f, .5f), 0.5f),
				new Sphere(Vec3(5.0f, 0.0f, -15.0f), 2, Vec3(.3f, .8f, .8f), 0.2f),
				new Sphere(Vec3(-5.0f, 0.0f, -15.0f), 2, Vec3(.3f, .5f, .8f), 0.2f),
				new Sphere(Vec3(-2.0f, -1.0f, -10.0f), 1, Vec3(.1f, .1f, .1f), 0.1f, 0.8f)
			};

			scene->LightCount = 1;
			scene->Lights = new Light*[1]{new Light(Vec3(-10, 20, 30), Vec3(2, 2, 2))};
			
			int pixelsLength = Width * Height * 3;
			char* pixels = new char[pixelsLength];

			// give the system a little time
			cout << "Give the system a little time..." << endl;
			#ifdef WIN32
			Sleep(2000);
			#else
			usleep(2000 * 1000);
			#endif
			cout << "Starting test..." << endl;

			// run test
			#if WIN32
			Win32OptimizedStopwatch();
			#endif

			unsigned int start = clock();
			auto data = Benchmark::Render(*scene, pixels);
            double sec = ((clock()-start) / (double)CLOCKS_PER_SEC);
			cout << "Sec: " << sec << endl;

			#if WIN32
			Win32EndOptimizedStopwatch();
			#endif

			//return sec;
			// save image
#if !IOS
			/*ofstream outfile("Image.rgb", ofstream::binary);
			outfile.write(data, pixelsLength);
			outfile.close();*/
#endif
            
            return sec;
		}
	};
}

#ifdef WIN32
int _tmain(int argc, wchar_t* argv[])
#elif IOS
double runTest()
#else
int main(int argc, char *argv[])
#endif
{
	double sec = RayTraceBenchmark::BenchmarkMain::Start();
#ifdef IOS
    return sec;
#endif
	return 0;
}