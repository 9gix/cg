#include "BVHAnimator.h"
#include "Eigen/Dense"
#include <ctime>

#include <iostream>
using namespace std;

// color used for rendering, in RGB
float color[3];

// convert a matrix to an array
static void mat4ToGLdouble16(GLdouble* m, glm::mat4 mat){
	// OpenGL matrix is column-major.
	for (uint i = 0; i < 4; i++)
		for (uint j = 0; j < 4; j++)
			m[i*4+j] = mat[i][j];
}

static glm::mat4 rigidToMat4(RigidTransform t) {
    glm::mat4 translation_mat = glm::translate(glm::mat4(1.0f), t.translation);
    glm::mat4 rotation_mat = glm::mat4_cast(t.quaternion);
    return translation_mat * rotation_mat;
}

static void rigidToGLdouble16(GLdouble *m, RigidTransform t) {
    mat4ToGLdouble16(m, rigidToMat4(t));
}

void renderSphere( float x, float y, float z, float size){		
	float radius = size;
	int numSlices = 32;
	int numStacks = 8; 
	static GLUquadricObj *quad_obj = gluNewQuadric();
	gluQuadricDrawStyle( quad_obj, GLU_FILL );
	gluQuadricNormals( quad_obj, GLU_SMOOTH );

	glPushMatrix();	
	glTranslated( x, y, z );

	glColor3f(color[0],color[1],color[2]);

	gluSphere(quad_obj,radius,numSlices,numStacks);	

	glPopMatrix();
}

/**
 * Draw a bone from (x0,y0,z0) to (x1,y1,z1) as a cylinder of radius (boneSize)
 */
void renderBone( float x0, float y0, float z0, float x1, float y1, float z1, float boneSize ){
	GLdouble  dir_x = x1 - x0;
	GLdouble  dir_y = y1 - y0;
	GLdouble  dir_z = z1 - z0;
	GLdouble  bone_length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );

	static GLUquadricObj *  quad_obj = NULL;
	if ( quad_obj == NULL )
		quad_obj = gluNewQuadric();	
	gluQuadricDrawStyle( quad_obj, GLU_FILL );
	gluQuadricNormals( quad_obj, GLU_SMOOTH );

	glPushMatrix();

	glTranslated( x0, y0, z0 );

	double  length;
	length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );
	if ( length < 0.0001 ) { 
		dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;
	}
	dir_x /= length;  dir_y /= length;  dir_z /= length;

	GLdouble  up_x, up_y, up_z;
	up_x = 0.0;
	up_y = 1.0;
	up_z = 0.0;

	double  side_x, side_y, side_z;
	side_x = up_y * dir_z - up_z * dir_y;
	side_y = up_z * dir_x - up_x * dir_z;
	side_z = up_x * dir_y - up_y * dir_x;

	length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );
	if ( length < 0.0001 ) {
		side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;
	}
	side_x /= length;  side_y /= length;  side_z /= length;

	up_x = dir_y * side_z - dir_z * side_y;
	up_y = dir_z * side_x - dir_x * side_z;
	up_z = dir_x * side_y - dir_y * side_x;

	GLdouble  m[16] = { side_x, side_y, side_z, 0.0,
	                    up_x,   up_y,   up_z,   0.0,
	                    dir_x,  dir_y,  dir_z,  0.0,
	                    0.0,    0.0,    0.0,    1.0 };
	glMultMatrixd( m );

	GLdouble radius= boneSize; 
	GLdouble slices = 8.0; 
	GLdouble stack = 3.0;  
	glColor3f(color[0],color[1],color[2]);
	gluCylinder( quad_obj, radius, radius, bone_length, slices, stack ); 

	glPopMatrix();
}

BVHAnimator::BVHAnimator(BVH* bvh)
{
	_bvh = bvh;
	setPointers();
}

void  BVHAnimator::renderFigure( int frame_no, float scale, int flag )
{	
	switch (flag){
	case 0:
		renderJointsGL( _bvh->getRootJoint(), _bvh->getMotionDataPtr(frame_no), scale);
	    break;
    case 1:
        renderJointsMatrix(frame_no, scale);
        break;
    case 2:
		renderJointsQuaternion(frame_no, scale);
		break;
	case 3:
		renderSkeleton( _bvh->getRootJoint(), _bvh->getMotionDataPtr(frame_no), scale );	
		break;
	case 4:	
		renderMannequin(frame_no,scale);
		break;
	default:
		break;
	}
}

void  BVHAnimator::renderSkeleton( const JOINT* joint, const float*data, float scale )
{	
	color[0] = 1.;
    color[1] = 0.;
    color[2] = 0.;

	float bonesize = 0.03;
	
    glPushMatrix();
	
    // translate
	if ( joint->parent == NULL )    // root
	{
		glTranslatef( data[0] * scale, data[1] * scale, data[2] * scale );
	}
	else
	{
		glTranslatef( joint->offset.x*scale, joint->offset.y*scale, joint->offset.z*scale );
	}

	// rotate
	for ( uint i=0; i<joint->channels.size(); i++ )
	{
		CHANNEL *channel = joint->channels[i];
		if ( channel->type == X_ROTATION )
			glRotatef( data[channel->index], 1.0f, 0.0f, 0.0f );
		else if ( channel->type == Y_ROTATION )
			glRotatef( data[channel->index], 0.0f, 1.0f, 0.0f );
		else if ( channel->type == Z_ROTATION )
			glRotatef( data[channel->index], 0.0f, 0.0f, 1.0f );
	}

	//end site? skip!
	if ( joint->children.size() == 0 && !joint->is_site)
	{
		renderBone(0.0f, 0.0f, 0.0f, joint->offset.x*scale, joint->offset.y*scale, joint->offset.z*scale,bonesize);
	}
	if ( joint->children.size() == 1 )
	{
		JOINT *  child = joint->children[ 0 ];
		renderBone(0.0f, 0.0f, 0.0f, child->offset.x*scale, child->offset.y*scale, child->offset.z*scale,bonesize);
	}
	if ( joint->children.size() > 1 )
	{
		float  center[ 3 ] = { 0.0f, 0.0f, 0.0f };
		for ( uint i=0; i<joint->children.size(); i++ )
		{
			JOINT *  child = joint->children[i];
			center[0] += child->offset.x;
			center[1] += child->offset.y;
			center[2] += child->offset.z;
		}
		center[0] /= joint->children.size() + 1;
		center[1] /= joint->children.size() + 1;
		center[2] /= joint->children.size() + 1;

		renderBone(	0.0f, 0.0f, 0.0f, center[0]*scale, center[1]*scale, center[2]*scale,bonesize);

		for ( uint i=0; i<joint->children.size(); i++ )
		{
			JOINT *  child = joint->children[i];
			renderBone(	center[0]*scale, center[1]*scale, center[2]*scale,
				child->offset.x*scale, child->offset.y*scale, child->offset.z*scale,bonesize);
		}
	}

	// recursively render all bones
	for ( uint i=0; i<joint->children.size(); i++ )
	{
		renderSkeleton( joint->children[ i ], data, scale );
	}
	glPopMatrix();
}

void  BVHAnimator::renderJointsGL( const JOINT* joint, const float*data, float scale )
{	
    // --------------------------------------
    // TODO: [Part 2a - Forward Kinematics]
    // --------------------------------------
    
	color[0] = 1.; color[1] = 0.; color[2] = 0.;

	glPushMatrix();

	// translate
	if ( joint->parent == NULL )    // root
	{
		glTranslatef( data[0] * scale, data[1] * scale, data[2] * scale );
	}
	else
	{
		glTranslatef( joint->offset.x*scale, joint->offset.y*scale, joint->offset.z*scale );
	}

	// rotate
	for (uint i = 0; i<joint->channels.size(); i++)
	{
		CHANNEL *channel = joint->channels[i];
		if (channel->type == X_ROTATION)
			glRotatef(data[channel->index], 1.0f, 0.0f, 0.0f);
		else if (channel->type == Y_ROTATION)
			glRotatef(data[channel->index], 0.0f, 1.0f, 0.0f);
		else if (channel->type == Z_ROTATION)
			glRotatef(data[channel->index], 0.0f, 0.0f, 1.0f);
	}

	// end site
	if ( joint->children.size() == 0 )
	{
		renderSphere(joint->offset.x*scale, joint->offset.y*scale, joint->offset.z*scale,0.07);
	}
	if ( joint->children.size() == 1 )
	{
		JOINT *  child = joint->children[ 0 ];
		renderSphere(child->offset.x*scale, child->offset.y*scale, child->offset.z*scale,0.07);
	}
	if ( joint->children.size() > 1 )
	{
		float  center[ 3 ] = { 0.0f, 0.0f, 0.0f };
		for ( uint i=0; i<joint->children.size(); i++ )
		{
			JOINT *  child = joint->children[i];
			center[0] += child->offset.x;
			center[1] += child->offset.y;
			center[2] += child->offset.z;
		}
		center[0] /= joint->children.size() + 1;
		center[1] /= joint->children.size() + 1;
		center[2] /= joint->children.size() + 1;

		renderSphere(center[0]*scale, center[1]*scale, center[2]*scale,0.07);

		for ( uint i=0; i<joint->children.size(); i++ )
		{
			JOINT *  child = joint->children[i];
			renderSphere(child->offset.x*scale, child->offset.y*scale, child->offset.z*scale,0.07);
		}
	}

	// recursively render all joints
	for ( uint i=0; i<joint->children.size(); i++ )
	{
		renderJointsGL( joint->children[i], data, scale );
	}
	glPopMatrix();
}


void  BVHAnimator::renderJointsMatrix( int frame, float scale )
{
	_bvh->matrixMoveTo(frame, scale);
	std::vector<JOINT*> jointList = _bvh->getJointList();	
	for(std::vector<JOINT*>::iterator it = jointList.begin(); it != jointList.end(); it++)
	{
		glPushMatrix();	
                
        GLdouble m[16];                  
		mat4ToGLdouble16(m, (*it)->matrix);
		glMultMatrixd(m);

		if ((*it)->is_site) {
			color[0] = 0.; color[1] = 1.; color[2] = 0.;
			renderSphere(0, 0, 0, 0.04);
		}
		else{
			color[0] = 0.; color[1] = 1.; color[2] = 0.;
			renderSphere(0, 0, 0, 0.07);
		}

		glPopMatrix();
	}	
}

void  BVHAnimator::renderJointsQuaternion( int frame, float scale )
{
	_bvh->quaternionMoveTo(frame, scale);
	std::vector<JOINT*> jointList = _bvh->getJointList();	
	for(std::vector<JOINT*>::iterator it = jointList.begin(); it != jointList.end(); it++)
	{
		glPushMatrix();	

        // convert quaternion and translation into matrix for rendering        
        glm::mat4 mat = rigidToMat4((*it)->transform);
        GLdouble m[16];                  
		mat4ToGLdouble16(m, mat);
		glMultMatrixd(m);

		if ((*it)->is_site) {
			color[0] = 0.; color[1] = 0.; color[2] = 1.;
			renderSphere(0, 0, 0, 0.04);
		} else {
			color[0] = 0.; color[1] = 0.; color[2] = 1.;
			renderSphere(0, 0, 0, 0.07);
		}

		glPopMatrix();
	}	
}

void BVHAnimator::setPointers(){
	head = _bvh->getJoint(std::string(_BVH_HEAD_JOINT_));
	neck = _bvh->getJoint(std::string(_BVH_NECK_JOINT_));
	chest = _bvh->getJoint(std::string(_BVH_CHEST_JOINT_));
	spine = _bvh->getJoint(std::string(_BVH_SPINE_JOINT_));
	hip = _bvh->getJoint(std::string(_BVH_ROOT_JOINT_));   // root joint

	lshldr = _bvh->getJoint(std::string(_BVH_L_SHOULDER_JOINT_));
	larm = _bvh->getJoint(std::string(_BVH_L_ARM_JOINT_));
	lforearm = _bvh->getJoint(std::string(_BVH_L_FOREARM_JOINT_));
	lhand = _bvh->getJoint(std::string(_BVH_L_HAND_JOINT_));

	rshldr = _bvh->getJoint(std::string(_BVH_R_SHOULDER_JOINT_));
	rarm = _bvh->getJoint(std::string( _BVH_R_ARM_JOINT_));
	rforearm = _bvh->getJoint(std::string(_BVH_R_FOREARM_JOINT_));
	rhand = _bvh->getJoint(std::string(_BVH_R_HAND_JOINT_));

	lupleg = _bvh->getJoint(std::string(_BVH_L_THIGH_JOINT_));
	lleg = _bvh->getJoint(std::string(_BVH_L_SHIN_JOINT_));
	lfoot = _bvh->getJoint(std::string(_BVH_L_FOOT_JOINT_));
	ltoe = _bvh->getJoint(std::string(_BVH_L_TOE_JOINT_));

	rupleg = _bvh->getJoint(std::string(_BVH_R_THIGH_JOINT_));
	rleg = _bvh->getJoint(std::string(_BVH_R_SHIN_JOINT_));
	rfoot = _bvh->getJoint(std::string(_BVH_R_FOOT_JOINT_));
	rtoe = _bvh->getJoint(std::string(_BVH_R_TOE_JOINT_));
}

void getScaledOffset(OFFSET& c, OFFSET a, OFFSET b, float scale)
{
	c.x = (a.x-b.x)*scale;
	c.y = (a.y-b.y)*scale;
	c.z = (a.z-b.z)*scale;
}

void BVHAnimator::renderMannequin(int frame, float scale) {

    // --------------------------------------
    // TODO: [Part 2c - Forward Kinematics]
    // --------------------------------------
	// You can draw a couple of basic geometries to build the mannequin 
    // using the renderSphere() and renderBone() provided in BVHAnimator.cpp 
    // or GL functions like glutSolidCube(), etc.
    
    _bvh->quaternionMoveTo(frame, scale);
    //_bvh->matrixMoveTo(frame, scale);
    // NOTE: you can use matrix or quaternion to calculate the transformation

	renderSkeleton(_bvh->getRootJoint(), _bvh->getMotionDataPtr(frame), scale);
	std::vector<JOINT*> jointList = _bvh->getJointList();
	for (std::vector<JOINT*>::iterator it = jointList.begin(); it != jointList.end(); it++)
	{
		JOINT* joint = (*it);
		glPushMatrix();

		// convert quaternion and translation into matrix for rendering        
		glm::mat4 mat = rigidToMat4((*it)->transform);
		GLdouble m[16];
		mat4ToGLdouble16(m, mat);
		glMultMatrixd(m);

		float r, g, b; r = g = b = 1;
		float xScale, yScale, zScale; xScale = yScale = zScale = 1;
		
		if (joint == head) {
			glPushMatrix();
			glScalef(1, 2, 1);
			color[0] = 1; color[1] = 0.7; color[2] = 0.1;
			renderSphere(0, 0, 0, 0.1);
			glPopMatrix();
		}
		else if (joint == neck) {
			glPushMatrix();
			color[0] = 0.1; color[1] = 0.1; color[2] = 0.3;
			renderSphere(0, 0, 0, 0.02);
			glPopMatrix();
		}
		else if (joint == chest) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 0.5; color[2] = 0.5;
			glScalef(1.1, 2.5, 0.5);
			glutSolidCube(0.2);
			glPopMatrix();
		}
		else if (joint == spine) {
			glPushMatrix();
			color[0] = 1; color[1] = 0.8; color[2] = 1;
			renderSphere(0, 0, 0, 0.05);
			glPopMatrix();
		}
		else if (joint == hip) {
			glPushMatrix();
			color[0] = 0.9; color[1] = 0.3; color[2] = 0.5;
			glScalef(1.5, 0.4, 0.5);
			renderSphere(0, 0, 0, 0.05);
			glPopMatrix();
		}
		else if (joint == lshldr || joint == rshldr) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 0.5; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.15);
			glPopMatrix();
		}
		else if (joint == larm || joint == rarm) {
			glPushMatrix();
			color[0] = 0.5; color[1] = 0.3; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.12);
			glPopMatrix();
		}
		else if (joint == lforearm || joint == rforearm) {
			glPushMatrix();
			color[0] = 0.8; color[1] = 0.6; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.09);
			glPopMatrix();
		}
		else if (joint == lhand || joint == rhand) {
			glPushMatrix();
			color[0] = 0.4; color[1] = 0.3; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.05);
			glPopMatrix();
		}
		else if (joint == lupleg || joint == rupleg) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 0.3; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.1);
			glPopMatrix();
		}
		else if (joint == lleg || joint == rleg) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 1; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.08);
			glPopMatrix();
		}
		else if (joint == lfoot || joint == rfoot) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 0.3; color[2] = 0.0;
			renderSphere(0, 0, 0, 0.02);
			glPopMatrix();
		}
		else if (joint == ltoe || joint == rtoe) {
			glPushMatrix();
			color[0] = 0.3; color[1] = 0.3; color[2] = 0.5;
			renderSphere(0, 0, 0, 0.05);
			glPopMatrix();
		}

		

		glPopMatrix();
	}
}

void BVHAnimator::solveLeftArm(int frame_no, float scale, float x, float y, float z)
{
    //_bvh->matrixMoveTo(frame_no, scale);      
    _bvh->quaternionMoveTo(frame_no, scale);
    // NOTE: you can use either matrix or quaternion to calculate the transformation

	float *LArx, *LAry, *LArz, *LFAry;
	
	float *mdata = _bvh->getMotionDataPtr(frame_no);
	// 3 channels - Xrotation, Yrotation, Zrotation
    // extract value address from motion data        
    CHANNEL *channel = larm->channels[0];
	LArx = &mdata[channel->index];
	channel = larm->channels[1];
	LAry = &mdata[channel->index];
	channel = larm->channels[2];
	LArz = &mdata[channel->index];

	channel = lforearm->channels[1];
	LFAry = &mdata[channel->index];
    
    cout << "Solving inverse kinematics..." << endl;
    clock_t start_time = clock();

    // -------------------------------------------------------
    // [Part 3] - Inverse Kinematics
    //
    // Discussion of the Algorithms with Gabriella Michelle
    // -------------------------------------------------------

	float xDiff = lforearm->offset.x * scale - larm->offset.x * scale;
	float yDiff = lforearm->offset.y * scale - larm->offset.y * scale;
	float zDiff = lforearm->offset.z * scale - larm->offset.z * scale;
	float armLen = sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);

	xDiff = lhand->offset.x * scale - lforearm->offset.x * scale;
	yDiff = lhand->offset.y * scale - lforearm->offset.y * scale;
	zDiff = lhand->offset.z * scale - lforearm->offset.z * scale;
	float handLen = sqrt(xDiff * xDiff + yDiff * yDiff + zDiff * zDiff);

	float limitErr = 1;
	int iter = 10; 
	while (iter > 0) { // Iteratively Validates the Delta between End Point and Goal.
		glm::vec4 curPos = (lhand->matrix) * glm::vec4(_bvh->getRootJoint()->offset.x, _bvh->getRootJoint()->offset.y, _bvh->getRootJoint()->offset.z, 1);

		// Analyticial Jacobian Matrix (Derivation of the FK)
		Eigen::MatrixXf j = Eigen::MatrixXf::Zero(3, 4);
		j(0, 0) = 0;
		j(0, 1) = armLen * cos(*LArz) * cos(*LAry) + handLen * cos(*LArz) * cos(*LAry + *LFAry);
		j(0, 2) = -sin(*LArz) * armLen * sin(*LAry) - sin(*LArz) * handLen * sin(*LAry + *LFAry);
		j(0, 3) = handLen * cos(*LArz) * cos(*LAry + *LFAry);

		j(1, 0) = -armLen * sin(*LArx) * sin(*LArz) - handLen * sin(*LArx) * sin(*LArz);
		j(1, 1) = 0;
		j(1, 2) = (armLen + handLen) * cos(*LArx) * cos(*LArz);
		j(1, 3) = 0;

		j(2, 0) = cos(*LArx) * (armLen * cos(*LAry) + handLen * cos(*LAry + *LFAry));
		j(2, 1) = sin(*LArx) * (armLen * -sin(*LAry) - handLen * sin(*LAry + *LFAry));
		j(2, 2) = 0;
		j(2, 3) = -handLen * sin(*LArx) * sin(*LAry + *LFAry);

		// Pseudo inverse of Jacobian
		Eigen::MatrixXf jacobianPseudoInverse = j.transpose() * (j * j.transpose()).inverse();

		// Compute Changes between the Goal and the End points
		Eigen::VectorXf dP = Eigen::VectorXf::Zero(3, 1);
		dP(0) = x - curPos[0];
		dP(1) = y - curPos[1];
		dP(2) = z - curPos[2];

		// Computing Changes in DOFs
		Eigen::VectorXf theta = jacobianPseudoInverse * dP;

		// Applying a small Step of theta 
		*LArx += theta(0);
		*LAry += theta(1);
		*LArz += theta(2);
		*LFAry += theta(3);

		cout << theta << endl;

		iter--;
	}

    // ----------------------------------------------------------
    // Do not touch
    // ----------------------------------------------------------
    clock_t end_time = clock();
    float elapsed = (end_time - start_time) / (float)CLOCKS_PER_SEC;
    cout << "Solving done in " << elapsed * 1000 << " ms." << endl;
}