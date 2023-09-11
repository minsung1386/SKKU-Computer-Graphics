#pragma once

struct sphere_t
{
	float	radius;
	float	dist_from_center;
	float	rotate_scale;
	float	revolve_scale;
	mat4	model_matrix;

	void	update(float theta, std::vector<sphere_t>& spheres);
	void	set_attribute(float rad, float dist, float rot_s, float rev_s);
	void	pause();
};

inline std::vector<sphere_t> create_spheres()
{
	std::vector<sphere_t> spheres;

	sphere_t s;
	// Sun
	s.set_attribute(0.7f, 0.0f, 0.03f, 0.0f);
	spheres.emplace_back(s);
	// Mercury
	s.set_attribute(0.08f, 1.2f, 1.2f, 2.1f);
	spheres.emplace_back(s);
	// Venus
	s.set_attribute(0.1f, 1.7f, 0.8f, 1.2f);
	spheres.emplace_back(s);
	// Earth
	s.set_attribute(0.21f, 2.4f, 0.6f, 0.83f);
	spheres.emplace_back(s);
	// Mars
	s.set_attribute(0.18f, 3.0f, 0.27f, 0.355f);
	spheres.emplace_back(s);
	// Jupiter
	s.set_attribute(0.38f, 5.1f, 0.15f, 0.262f);
	spheres.emplace_back(s);
	// Saturn
	s.set_attribute(0.345f, 7.9f, 0.17f, 0.35f);
	spheres.emplace_back(s);
	// Uranus
	s.set_attribute(0.25f, 8.8f, 0.12f, 0.34f);
	spheres.emplace_back(s);
	// Neptune
	s.set_attribute(0.23f, 9.4f, 0.12f, 0.2f);
	spheres.emplace_back(s);


	return spheres;
}

inline void sphere_t::update(float theta, std::vector<sphere_t>& spheres)
{
	float rotate_theta = theta * rotate_scale;
	float revolve_theta = theta * revolve_scale;

	mat4 scale_matrix = mat4::scale(radius);
	model_matrix = mat4::rotate(vec3(0, 0, 1), revolve_theta) * mat4::translate(vec3(dist_from_center, 0, 0)) * mat4::rotate(vec3(0, 0, 1), rotate_theta) * scale_matrix;
}

inline void	sphere_t::set_attribute(float rad, float dist, float rot_s, float rev_s)
{
	radius = rad;
	dist_from_center = dist;
	rotate_scale = rot_s;
	revolve_scale = rev_s;
}

void sphere_t::pause()
{

}