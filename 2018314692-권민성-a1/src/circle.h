#pragma once
#ifndef __CIRCLE_H__
#define __CIRCLE_H__

struct circle_t
{
	vec2	center=vec2(0);		// 2D position for translation
	float	radius;				// radius
	vec4	color;				// RGBA color in [0,1]
	mat4	model_matrix;		// modeling transformation
	vec2	velocity=vec2(0);			// ¼Óµµ
	float	mass;
	float	circle_time = 0.0f;

	// public functions	
	void	update(std::vector<circle_t>& circles, int circle_cnt, float t);
	void	collision(std::vector<circle_t>& circles, int circle_cnt);
};

inline std::vector<circle_t> create_circles(int seed, int cnt)
{
	std::vector<circle_t> circles;
	// get random attribs
	float x_max = 1.6f, x_min = -1.6f;
	float y_max = 0.9f, y_min = -0.9f;
	float radius_max = 1.0f/(float)sqrt(cnt), radius_min = 0.3f/ (float)sqrt(cnt);
	float vel_max = 0.002f, vel_min = -0.002f;
	bool overlap_flag;
	srand(seed);
	for (int i = 0; i < cnt; i++) {
		overlap_flag = false;
		circle_t c;
		float x = (float(rand()) / float(RAND_MAX)) * (x_max-x_min) + x_min;
		float y = (float(rand()) / float(RAND_MAX)) * (y_max-y_min) + y_min;
		float rad = (float(rand()) / float(RAND_MAX)) * (radius_max - radius_min) + radius_min;
		float cr = float(rand()) / float(RAND_MAX);
		float cg = float(rand()) / float(RAND_MAX);
		float cb = float(rand()) / float(RAND_MAX);
		float ca = float(rand()) / float(RAND_MAX);
		vec2 vel = vec2((float(rand()) / float(RAND_MAX)) * (vel_max - vel_min) + vel_min, (float(rand()) / float(RAND_MAX)) * (vel_max - vel_min) + vel_min);
		c = { vec2(x,y), rad, vec4(cr,cg,cb,ca) };
		c.velocity = vel;
		c.mass = rad * rad;

		// avoid overlapping between circles : O(N^2)
		for (int j = 0; j < i; j++) {
			float dist = (circles[j].center.x - x) * (circles[j].center.x - x) + (circles[j].center.y - y) * (circles[j].center.y - y);
			if (sqrt(dist) > rad + circles[j].radius) overlap_flag = false;
			else {
				overlap_flag = true;
				break;
			}
		}
		if (overlap_flag) i--;
		else circles.emplace_back(c);
	}
	return circles;
}

inline void circle_t::update(std::vector<circle_t>& circles, int circle_cnt, float t )
{
	// handle collision
	collision(circles, circle_cnt);
	// move each circles
	float interval = (t - circle_time)*60;	// 60 fps
	circle_time=t;
	center += velocity/interval;		

	// these transformations will be explained in later transformation lecture
	mat4 scale_matrix =
	{
		radius, 0, 0, 0,
		0, radius, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 rotation_matrix =
	{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1
	};

	mat4 translate_matrix =
	{
		1, 0, 0, center.x,
		0, 1, 0, center.y,
		0, 0, 1, 0,
		0, 0, 0, 1
	};
	
	model_matrix = translate_matrix*rotation_matrix*scale_matrix;
}

inline void circle_t::collision(std::vector<circle_t>& circles, int circle_cnt) {
	// collision with wall
	if (center.x + radius > 1.77777f || center.x - radius < -1.77777f)
	{
		// check next frame
		vec2 next_center = center + velocity;
		//getting worse
		if ((center.x > 0 && next_center.x > center.x) || (center.x < 0 && next_center.x < center.x))	
		{
			velocity = vec2(-velocity.x, velocity.y);
		}
	}
	if (center.y + radius > 1.0f || center.y - radius < -1.0f) 
	{
		vec2 next_center = center + velocity;
		if ((center.y > 0 && next_center.y > center.y) || (center.y < 0 && next_center.y < center.y))
		{
			velocity = vec2(velocity.x, -velocity.y);
		}
	}

	// collistion with circles
	for (int i = 0; i < circle_cnt; i++) {
		float dist = (circles[i].center.x - center.x) * (circles[i].center.x - center.x) + (circles[i].center.y - center.y) * (circles[i].center.y - center.y);
		if (dist == 0) continue;
		// handle collision
		if (sqrt(dist) < circles[i].radius + radius)
		{
			vec2 next_center = center + velocity;
			vec2 next_center2 = circles[i].center + circles[i].velocity;
			float next_frame_dist = (next_center2.x - next_center.x) * (next_center2.x - next_center.x) + (next_center2.y - next_center.y) * (next_center2.y - next_center.y);
			if (next_frame_dist < dist)
			{
				vec2 contact_angle = center - circles[i].center;
				vec2 vel_diff = velocity - circles[i].velocity;
				velocity = velocity - ((2 * circles[i].mass) / (circles[i].mass + mass)) * (vel_diff.dot(contact_angle)) / (contact_angle.length2()) * contact_angle;
				circles[i].velocity = circles[i].velocity - ((2 * mass) / (circles[i].mass + mass)) * ((-vel_diff).dot(-contact_angle)) / (contact_angle.length2()) * (-contact_angle);
			}
			
		}
	}
}
#endif
