#include "bodyTypes.h"

struct Routes* copy_routes(struct Routes* routes, struct ProcessState* processState) {
	struct Routes* newRoutes = (struct Routes*)malloc(sizeof(struct Routes));
	newRoutes->routes = (struct Route**)malloc(sizeof(struct Route*) * routes->numberOfRoutes);
	newRoutes->numberOfRoutes = routes->numberOfRoutes;

	for (int i = 0; i < routes->numberOfRoutes; i++) {
		newRoutes->routes[i] = copy_route(routes->routes[i], processState);
	}

	return newRoutes;
}