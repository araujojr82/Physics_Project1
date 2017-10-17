INFO6019 Physics & Simulation Project 1

Authors: Euclides Araujo
         Jorge Amengol

User input keys:
[LIGHT CONTROL]
Backspace: Turn the selected light on/off;
C Key: Change the colour for the selected light;
,(<): decrease the linear attenuation for selected light;
.(>): increase the linear attenuation for selected light;

[CAMERA CONTROL]
Enter: Turn the wireframe mode on/off;
W,A,S,D,Q,E: Controls the camera position;

[BALL CONTROL]
Left and Right: Control the angle of the shot;
Up and Down: Control de velocity to be applied;
Space: Shoot the ball


Config is divided in 3 files, the config.txt has the interface options, 
while the meshlist.txt. and objects.txt holds the meshes and objects, 
both uses spaces to separate the properties.



meshlist.txt is composed of 1 mesh per line, and the line structure is:
meshname  name_of_the_mesh_file1.ply

example:
virus1    virus_super_low_res_XYZ.ply
asteroid1 asteroid_xyz.ply



objects.txt is composed of lines for each meshtipe, 
each line can create 1 object in the game, the line structure is:
meshname posX posY posZ scale TYPEOFOBJECT colourR colourG colourB colourA Mass line_description

example:
poolsides 0.0  0.0   0.0  1.0 PLANE  0.0  false 1.0  1.0  0.0  1.0  0.0          The_table_itself
ball    -11.0  0.26  1.0  1.0 SPHERE 0.26 true  1.0  1.0  1.0  1.0  471.0        The_white_ball