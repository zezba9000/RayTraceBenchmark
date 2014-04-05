#ifdef WIN32
#include "../RayTraceBenchmark/RayTraceBenchmark/stdafx.h"
#include <Windows.h>
#endif

#if !WIN32
#include <unistd.h>
#endif

#include <math.h>
#include <float.h>
#include <ctime>
#include <fstream>
#include <iostream>
using namespace std;

//#define BIT64

#if BIT64
#define Num double
#else
#define Num float
#endif

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

		Vec3 operator+(const Vec3& p)
		{
			return Vec3(X + p.X, Y + p.Y, Z + p.Z);
		}

		Vec3 operator+=(const Vec3& p)
		{
			this->X += p.X;
			this->Y += p.Y;
			this->Z += p.Z;
			return *this;
		}

		Vec3 operator-(const Vec3& p)
		{
			return Vec3(X - p.X, Y - p.Y, Z - p.Z);
		}

		Vec3 operator-()
		{
			return Vec3(-X, -Y, -Z);
		}

		Vec3 operator*(const Vec3& p)
		{
			return Vec3(X * p.X, Y * p.Y, Z * p.Z);
		}

		Vec3 operator*(const float p)
		{
			return Vec3(X * p, Y * p, Z * p);
		}

		Vec3 operator/(const Vec3& p)
		{
			return Vec3(X / p.X, Y / p.Y, Z / p.Z);
		}

		Vec3 operator/(const float p)
		{
			return Vec3(X / p, Y / p, Z / p);
		}

		static Num Dot(Vec3 v1, Vec3 v2)
		{
			return (v1.X*v2.X) + (v1.Y*v2.Y) + (v1.Z*v2.Z);
		}

		static Num Magnitude(Vec3 v)
		{
			return sqrt((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z));
		}

		static Vec3 Normalize(Vec3 v)
		{
			return v / sqrt((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z));
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

			auto c = (Num)sqrt(r2 - b2);
			auto _near = a - c;
			auto _far  = a + c;
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
		const static int Width = 1280;
		const static int Height = 720;
		#define fov 45
		#define maxDepth 6
		#define PI 3.1415926535897932384626433832795

		static Vec3 trace (Ray ray, const Scene& scene, int depth)
		{
			auto nearest = FLT_MAX;
			Sphere* obj = 0;

			// search the scene for nearest intersection
			for (int i = 0; i != scene.ObjectCount; ++i)
			{
				auto distance = FLT_MAX;
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
				r.Org = point_of_hit + (normal * 1e-5f);
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
					color += l->Color
						* max(0.0f, Vec3::Dot(normal, light_direction))
						* obj->Color
						* (1.0f - reflection_ratio);
				}
			}

			auto rayNormDot = Vec3::Dot(ray.Dir, normal);
			Num facing = max(0.0f, -rayNormDot);
			Num fresneleffect = reflection_ratio + ((1 - reflection_ratio) * pow((1 - facing), 5));

			// compute reflection
			if (depth < maxDepth && reflection_ratio > 0)
			{
				auto reflection_direction = ray.Dir + (normal * 2 * rayNormDot * (-1.0f));
				Ray r;
				r.Org = point_of_hit + (normal * 1e-5f);
				r.Dir = reflection_direction;
				auto reflection = trace(r, scene, depth + 1);
				color += reflection * fresneleffect;
			}

			// compute refraction
			if (depth < maxDepth && (obj->Transparency > 0))
			{
				auto ior = 1.5f;
				auto CE = Vec3::Dot(ray.Dir, normal) * (-1.0f);
				ior = inside ? (1.0f) / ior : ior;
				auto eta = (1.0f) / ior;
				auto GF = (ray.Dir + normal * CE) * eta;
				auto sin_t1_2 = 1 - (CE * CE);
				auto sin_t2_2 = sin_t1_2 * (eta * eta);
				if (sin_t2_2 < 1)
				{
					auto GC = normal * sqrt(1 - sin_t2_2);
					auto refraction_direction = GF - GC;
					Ray r;
					r.Org = point_of_hit - (normal * 1e-4f);
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
			Num h = tan(((fov / 360.0f) * (2.0f * PI)) / 2.0f) * 2.0f;
			Num w = h * Width / Height;

			for (int y = 0; y != Height; ++y)
			{
				for (int x = 0; x != Width; ++x)
				{
					Num xx = x, yy = y, ww = Width, hh = Height;
					Vec3 dir;
					dir.X = ((xx - (ww / 2.0f)) / ww)  * w;
					dir.Y = (((hh/2.0f) - yy) / hh) * h;
					dir.Z = -1.0f;
					dir = Vec3::Normalize(dir);

					Ray r;
					r.Org = eye;
					r.Dir = dir;
					auto pixel = trace(r, scene, 0);
					int i = (x*3) + (y*Width*3);
					pixels[i] = min(pixel.X * 255, 255.0f);
					pixels[i+1] = min(pixel.Y * 255, 255.0f);
					pixels[i+2] = min(pixel.Z * 255, 255.0f);
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

		static void Start()
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
			
			int pixelsLength = Benchmark::Width * Benchmark::Height * 3;
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
			cout << "Sec: " << ((clock()-start) / 1000.0) << endl;

			#if WIN32
			Win32EndOptimizedStopwatch();
			#endif

			// save image
			ofstream outfile ("Image.raw", ofstream::binary);
			outfile.write(data, pixelsLength);
			outfile.close();
		}
	};
}

#ifdef WIN32
int _tmain(int argc, wchar_t* argv[])
#else
int main(int argc, char *argv[])
#endif
{
	RayTraceBenchmark::BenchmarkMain::Start();
	return 0;
}