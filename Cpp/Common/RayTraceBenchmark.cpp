#ifndef SOMETHING
#define SOMETHING

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

#if IOS
#include "RayTraceBenchmark.cpp"
#endif

//#define BIT64
//#define USE_OUT

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

		#ifdef USE_OUT
		static void Add(const Vec3& p, const Vec3& p2, Vec3* result)
		{
			result->X = p.X + p2.X;
			result->Y = p.Y + p2.Y;
			result->Z = p.Z + p2.Z;
		}

		static void Sub(const Vec3& p, const Vec3& p2, Vec3* result)
		{
			result->X = p.X - p2.X;
			result->Y = p.Y - p2.Y;
			result->Z = p.Z - p2.Z;
		}

		Vec3 operator-()
		{
			return Vec3(-X, -Y, -Z);
		}

		static void Mul(const Vec3& p, const Vec3& p2, Vec3* result)
		{
			result->X = p.X * p2.X;
			result->Y = p.Y * p2.Y;
			result->Z = p.Z * p2.Z;
		}

		static void Mul(const Vec3& p, const Num p2, Vec3* result)
		{
			result->X = p.X * p2;
			result->Y = p.Y * p2;
			result->Z = p.Z * p2;
		}

		static void Div(const Vec3& p, const Vec3& p2, Vec3* result)
		{
			result->X = p.X / p2.X;
			result->Y = p.Y / p2.Y;
			result->Z = p.Z / p2.Z;
		}

		static void Div(const Vec3& p, const Num p2, Vec3* result)
		{
			result->X = p.X / p2;
			result->Y = p.Y / p2;
			result->Z = p.Z / p2;
		}

		static void Dot(Vec3 v1, Vec3 v2, Num* result)
		{
			*result = (v1.X*v2.X) + (v1.Y*v2.Y) + (v1.Z*v2.Z);
		}

		static void Magnitude(Vec3 v, Num* result)
		{
			*result = sqrt((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z));
		}

		static void Normalize(Vec3 v, Vec3* result)
		{
			Div(v, (Num)sqrt((v.X*v.X) + (v.Y*v.Y) + (v.Z*v.Z)), result);
		}
		#else
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
		#endif
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
		
		#ifdef USE_OUT
		static void Normal(Sphere& sphere, Vec3 pos, Vec3* result)// <<<<<<<<<<<<<<<<<< TODO USE_OUT <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
		{
			Vec3 subResult;
			Vec3::Sub(pos, sphere.Center, &subResult);
			Vec3::Normalize(subResult, result);
		}

		static void Intersect(Sphere& sphere, Ray ray, bool* result)
		{
			Vec3 l;
			Vec3::Sub(sphere.Center, ray.Org, &l);
			Num a;
			Vec3::Dot(l, ray.Dir, &a);
			if (a < 0)              // opposite direction
			{
				*result = false;
				return;
			}

			Num dotResult;
			Vec3::Dot(l, l, &dotResult);
			auto b2 = dotResult - (a * a);
			auto r2 = sphere.Radius * sphere.Radius;
			if (b2 > r2)            // perpendicular > r
			{
				*result = false;
				return;
			}

			*result = true;
		}

		static void Intersect(Sphere& sphere, Ray ray, Num* distance, bool* result)
		{
			*distance = 0;

			Vec3 l;
			Vec3::Sub(sphere.Center, ray.Org, &l);
			Num a;
			Vec3::Dot(l, ray.Dir, &a);
			if (a < 0)              // opposite direction
			{
				*result = false;
				return;
			}

			Num dotResult;
			Vec3::Dot(l, l, &dotResult);
			auto b2 = dotResult - (a * a);
			auto r2 = sphere.Radius * sphere.Radius;
			if (b2 > r2)            // perpendicular > r
			{
				*result = false;
				return;
			}

			auto c = (Num)sqrt(r2 - b2);
			auto _near = a - c;
			auto _far  = a + c;
			*distance = (_near < 0) ? _far : _near;
			// near < 0 means ray starts inside
			*result = true;
		}
		#else
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
		#endif
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

		#ifdef USE_OUT
		static void trace (Ray ray, const Scene& scene, int depth, Vec3* result)
		{
			auto nearest = FLT_MAX;
			Sphere* obj = 0;

			// search the scene for nearest intersection
			for (int i = 0; i != scene.ObjectCount; ++i)
			{
				auto distance = FLT_MAX;
				auto o = scene.Objects[i];
				bool boolResult;
				Sphere::Intersect(*o, ray, &distance, &boolResult);
				if (boolResult)
				{
					if (distance < nearest)
					{
						nearest = distance;
						obj = o;
					}
				}
			}

			if (obj == 0)
			{
				*result = Vec3();
				return;
			}

			Vec3 mulResult;
			Vec3::Mul(ray.Dir, nearest, &mulResult);
			Vec3 point_of_hit;
			Vec3::Add(ray.Org, mulResult, &point_of_hit);
			Vec3 normal;
			Sphere::Normal(*obj, point_of_hit, &normal);
			bool inside = false;

			Num dotResult;
			Vec3::Dot(normal, ray.Dir, &dotResult);
			if (dotResult > 0)
			{
				inside = true;
				normal = -normal;
			}

			Vec3 color = Vec3();
			auto reflection_ratio = obj->Reflection;

			for (int i = 0; i != scene.LightCount; ++i)
			{
				auto l = scene.Lights[i];
				Vec3 light_direction;
				Vec3 subResult;
				Vec3::Sub(l->Position, point_of_hit, &subResult);
				Vec3::Normalize(subResult, &light_direction);
				Ray r;
				Vec3 mulResult2;
				Vec3::Mul(normal, 1e-5f, &mulResult2);
				Vec3::Add(point_of_hit, mulResult2, &r.Org);
				r.Dir = light_direction;

				// go through the scene check whether we're blocked from the lights
				bool blocked = false;
				for (int i = 0; i != scene.ObjectCount; ++i)
				{
					auto o = scene.Objects[i];
					bool boolResult;
					Sphere::Intersect(*o, r, &boolResult);
					if (boolResult)
					{
						blocked = true;
						break;
					}
				}

				if (!blocked)
				{
					Num dotResult2;
					Vec3::Dot(normal, light_direction, &dotResult2);
					Vec3 colorMulResult;
					Vec3::Mul(l->Color, max(0, dotResult2), &colorMulResult);
					Vec3 colorMulResult2;
					Vec3::Mul(colorMulResult, obj->Color, &colorMulResult2);
					Vec3 colorMulResult3;
					Vec3::Mul(colorMulResult2, (1.0f - reflection_ratio), &colorMulResult3);
					Vec3::Add(color, colorMulResult3, &color);
				}
			}

			Num rayNormDot;
			Vec3::Dot(ray.Dir, normal, &rayNormDot);
			Num facing = max(0.0f, -rayNormDot);
			Num fresneleffect = reflection_ratio + ((1 - reflection_ratio) * pow((1 - facing), 5));

			// compute reflection
			if (depth < maxDepth && reflection_ratio > 0)
			{
				Vec3 normMulResult;
				Vec3::Mul(normal, 2, &normMulResult);
				Vec3 rayNormMulResult;
				Vec3::Mul(normMulResult, rayNormDot, &rayNormMulResult);
				Vec3 rayNormMulResult2;
				Vec3::Mul(rayNormMulResult, -1.0f, &rayNormMulResult2);
				Vec3 reflection_direction;
				Vec3::Add(ray.Dir, rayNormMulResult2, &reflection_direction);
				Ray r;
				Vec3 mulResult2;
				Vec3::Mul(normal, 1e-5f, &mulResult2);
				Vec3::Add(point_of_hit, mulResult2, &r.Org);
				r.Dir = reflection_direction;
				Vec3 reflection;
				trace(r, scene, depth + 1, &reflection);
				Vec3 colorMul;
				Vec3::Mul(reflection, fresneleffect, &colorMul);
				Vec3::Add(color, colorMul, &color);
			}

			// compute refraction
			if (depth < maxDepth && (obj->Transparency > 0))
			{
				auto ior = 1.5f;
				Num dotResult2;
				Vec3::Dot(ray.Dir, normal, &dotResult2);
				auto CE = dotResult2 * (-1.0f);
				ior = inside ? (1.0f) / ior : ior;
				auto eta = (1.0f) / ior;
				Vec3 ceMulResult;
				Vec3::Mul(normal, CE, &ceMulResult);
				Vec3 ceAddResult;
				Vec3::Add(ray.Dir, ceMulResult, &ceAddResult);
				Vec3 GF;
				Vec3::Mul(ceAddResult, eta, &GF);
				auto sin_t1_2 = 1 - (CE * CE);
				auto sin_t2_2 = sin_t1_2 * (eta * eta);
				if (sin_t2_2 < 1)
				{
					Vec3 GC;
					Vec3::Mul(normal, (Num)sqrt(1 - sin_t2_2), &GC);
					Vec3 refraction_direction;
					Vec3::Sub(GF, GC, &refraction_direction);
					Ray r;
					Vec3 refraction;
					trace(r, scene, depth + 1, &refraction);
					Vec3 refMulResult;
					Vec3::Mul(refraction, (1 - fresneleffect), &refMulResult);
					Vec3 mulResult4;
					Vec3::Mul(refMulResult, obj->Transparency, &mulResult4);
					Vec3::Add(color, mulResult4, &color);
				}
			}

			*result = color;
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
					dir.X = ((xx - (ww / 2.0f)) / ww) * w;
					dir.Y = (((hh/2.0f) - yy) / hh) * h;
					dir.Z = -1.0f;
					Vec3::Normalize(dir, &dir);

					Ray r;
					r.Org = eye;
					r.Dir = dir;
					Vec3 pixel;
					trace(r, scene, 0, &pixel);
					int i = (x*3) + (y*Width*3);
					pixels[i] = min(pixel.X * 255, 255.0f);
					pixels[i+1] = min(pixel.Y * 255, 255.0f);
					pixels[i+2] = min(pixel.Z * 255, 255.0f);
				}
			}

			return pixels;
		}
		#else
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
					dir.X = ((xx - (ww / 2.0f)) / ww) * w;
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
		#endif
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
            double sec = ((clock()-start) / (double)CLOCKS_PER_SEC);
			cout << "Sec: " << sec << endl;

			#if WIN32
			Win32EndOptimizedStopwatch();
			#endif

			// save image
#if !IOS
			ofstream outfile ("Image.rgb", ofstream::binary);
			outfile.write(data, pixelsLength);
			outfile.close();
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

#endif