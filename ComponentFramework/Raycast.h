#pragma once

#include <VMath.h>
#include <Matrix.h>
#include <glew.h>
#include <iostream>
#include <SDL.h>
#include <MMath.h>

namespace MATH {

	class Raycast {

	public:

		///Gets the direction of mouseclick on the screen from the camera 
		static Vec3 screenRayCast(double xpos, double ypos, Matrix4 projection, Matrix4 view) {


			int w, h;

			SDL_GetWindowSize(SDL_GL_GetCurrentWindow(), &w, &h);

			// converts a position from the 2d xpos, ypos to a normalized 3d direction
			float x = (2.0f * xpos) / w - 1.0f;
			float y = 1.0f - (2.0f * ypos) / h;
			float z = 1.0f;
			Vec3 ray_nds = Vec3(x, y, z);
			Vec4 ray_clip = Vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
			// eye space to clip we would multiply by projection so
			// clip space to eye space is the inverse projection
			Vec4 ray_eye = MMath::inverse(projection) * ray_clip;
			// convert point to forwards
			ray_eye = Vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);
			// world space to eye space is usually multiply by view so
			// eye space to world space is inverse view
			Vec4 inv_ray_wor = (MMath::inverse(view) * ray_eye);
			Vec3 ray_wor = Vec3(inv_ray_wor.x, inv_ray_wor.y, inv_ray_wor.z);
			ray_wor = VMath::normalize(ray_wor);
			return ray_wor;
		}

		struct Triangle {
			Vec3 v0, v1, v2;

			Vec3 getCenter() {return ((v0 + v1 + v2) / 3.0f);}

		};
		
		/// Creates a cone from a ray and checks if the targetPos is inside
		static bool isInRayCone(Vec3 targetPos, Vec3 rayOrigin, Vec3 rayDir, float cosMaxAngle) {

			Vec3 toPoint = targetPos - rayOrigin;
			float len = VMath::mag(toPoint);

			if (len < 1e-6f) {
				return true;
			}

			toPoint = toPoint / len;  

			return VMath::dot(rayDir, toPoint) >= cosMaxAngle;
		}

		/// <summary>
		/// Checks if a ray is intersecting a triangle. (Expensive)
		/// </summary>
		/// <param name="tri">= Triangle to test intersection</param>
		/// <param name="o">= Ray start</param>
		/// <param name="n">= Ray direction</param>
		/// <returns></returns>
		static bool intersectRayTri(const Triangle& tri, Vec3 o, Vec3 n) {
			// Ensure ray direction is normalized
			n = VMath::normalize(n);

			Vec3 e1 = tri.v1 - tri.v0;
			Vec3 e2 = tri.v2 - tri.v0;

			Vec3 pvec = VMath::cross(n, e2);

			float det = VMath::dot(pvec, e1);

			// "If the determinant is close to 0, the ray lies in the plane of the triangle" -someone smarter than me
			if (fabs(det) < 1e-8f)
				return false;

			float invDet = 1.0f / det;
			Vec3 tvec = o - tri.v0;
			float u = invDet * VMath::dot(tvec, pvec);
			if (u < 0.0f || u > 1.0f) return false;

			Vec3 qvec = VMath::cross(tvec, e1);
			float v = invDet * VMath::dot(qvec, n);
			if (v < 0.0f || u + v > 1.0f) return false;


			//check if intersection is in the opposite direction
			float t = invDet * VMath::dot(e2, qvec);
			if (t < 0.0f) return false; 

			/*Vec3 hit = o + n * t;
			std::cout << "Ray hit triangle at: ("
				<< hit.x << ", " << hit.y << ", " << hit.z << ")\n";*/

			return true;
		}

	};

}