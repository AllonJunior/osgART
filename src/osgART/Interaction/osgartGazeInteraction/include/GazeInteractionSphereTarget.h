#ifndef GAZEINTERACTIONSPHERETARGET_H
#define GAZEINTERACTIONSPHERETARTET_H

#include "GazeInteractionTarget.h"


class GazeInteractionSphereTarget : public GazeInteractionTarget
{
	public:    

		GazeInteractionSphereTarget();
		~GazeInteractionSphereTarget();
    
		bool setRadius(int radius);

		GazeInteractionTarget::GITargetType getType();
		int getRadius();

	protected:
	
		
		int m_radius;

	private:
};
#endif