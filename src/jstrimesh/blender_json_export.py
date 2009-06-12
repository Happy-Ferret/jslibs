#!BPY
""" Registration info for Blender menus:
Name: 'JSON format (.json)...'
Blender: 239
Group: 'Export'
Submenu: 'All Objects...' all
Submenu: 'Selected Objects...' selected
Tooltip: 'Export to my format (.json)'
"""

__author__ = ("soubok@gmail.com")
__url__ = ["http://jslibs.googlecode.com/"]
__version__ = "2008-10-30"
__bpydoc__ = """\

"""

################################################################

import Blender
import BPyMesh
import sys

from os.path import exists, join


fileExtension = 'json'


def exportToFile(filename):
	#object: http://www.blender.org/modules/documentation/237PythonDoc/Object.Object-class.html

	start = Blender.sys.time()
	if not filename.lower().endswith('.'+fileExtension):
		filename += '.json'

	currentWorld = Blender.World.GetCurrent()

	selectedObjectList = Blender.Object.GetSelected()

	if len(selectedObjectList) == 0:
		print "no selection"

	file = open(filename,"w")
	
	def Write(text):
		file.write( text )

	def WriteLn(text):
		Write( text + '\n' )


	Write( '{' );
	for o in selectedObjectList:
	
		# type: 'Armature', 'Camera', 'Curve', 'Lamp', 'Lattice', 'MBall', 'Mesh', 'Surf', 'Empty', 'Wave' (deprecated) or 'unknown' in exceptional cases. 
		if o.getType() == "Empty":
			Write( '"'+o.name+'":{"type":"'+o.getType()+'","parent":"'+str(o.parent)+'","position":[%s,%s,%s]' % o.loc + '},' );
		
		if o.getType() == "Mesh":
			Write( '"'+o.name+'":{"type":"'+o.getType()+'","parent":"'+str(o.parent)+'","position":[%s,%s,%s]' % o.loc + ',' );

			Write( '"mass":%s,' % o.rbMass ); # try: getSBMass()

			bb = o.getBoundBox(); # list of 8 (x,y,z) float coordinate vectors (WRAPPED DATA)
#			size = [ bb[0][n]+(bb[-2][n]-bb[0][n]) for n in [0,1,2] ]

			Write( '"box":[[%s,%s,%s],[%s,%s,%s]],' % (bb[0][0],bb[0][1],bb[0][2], bb[-2][0],bb[-2][1],bb[-2][2]) );
			
			Write( '"rotation":[%s,%s,%s],' % (o.RotX, o.RotY, o.RotZ) );
			Write( '"scale":[%s,%s,%s],' % o.size );

			Write( '"drawType":%s,' % o.drawType ); # 1 - Bounding box, 2 - wire, 3 - Solid, 4- Shaded, 5 - Textured. 

			Write( '"vertex":{');
			for v in o.data.verts:
				Write('"%s":[%s,%s,%s,%s,%s,%s,%s,%s,0],' % (v.index, v.co[0],v.co[1],v.co[2],v.no[0],v.no[1],v.no[2],v.uvco[0],v.uvco[1])) # index: x, y, z, normal.x, normal.y, normal.z, u, v
			Write( '},' );

			Write( '"face":[' );
			for f in o.data.faces:
				faceData = '%s,%s,%s,%s,%s' % (f.no[0], f.no[1], f.no[2], f.materialIndex, f.smooth ) # , f.image.name

				if len(f.v) == 4:
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[1].index, f.v[2].index) + faceData +']' )
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[2].index, f.v[3].index) + faceData +']' )
				else:
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[1].index, f.v[2].index) + faceData +']' )
			Write( '],' );


			#material: http://www.blender.org/modules/documentation/237PythonDoc/Material.Material-class.html#specTransp
			
			materialCount = len(o.data.materials)
#			WriteLn('#materialCount')
#			WriteLn( str(materialCount) )

			Write('"material":[')
			for i in range(0,materialCount,1):
				m = o.data.materials[i]

				Write( '[' );
				#ambiant: r g b ambientFactor
				Write('%s,%s,%s,%s' % (currentWorld.amb[0],currentWorld.amb[1],currentWorld.amb[2], m.amb) )

				#diffuse: r g b alpha
				Write(',%s,%s,%s,%s' % (m.rgbCol[0], m.rgbCol[1], m.rgbCol[2], m.alpha) )

				#specular: r g b specularTransparency[0.0,2.0] specularity[0.0,2.0] hardness[1,255]
				Write(',%s,%s,%s,%s,%s,%s' % (m.specCol[0],m.specCol[1],m.specCol[2], m.specTransp, m.spec, m.hard) )

				#emission: r g b emittingLightIntensity[0.0,1.0]
				Write(',%s,%s,%s,%s' % (m.specCol[0],m.specCol[1],m.specCol[2], m.emit) )
				Write( ']' );
				
				# miss .mode ?

			Write( '],' );
			Write('},');
		
	Write( '}' );

	end = Blender.sys.time()
	message = 'Successfully exported "%s" in %.4f seconds' % ( Blender.sys.basename(filename), end-start)
	print message



#def select_file(filename):
#	if exists(filename):
#		result = Blender.Draw.PupMenu("File Already Exists, Overwrite?%t|Yes%x1|No%x0")
#		if(result != 1):
#			return
#	if filename.find('.' + fileExtension, -4) < 0:
#		filename += '.' + fileExtension
#	exportToFile(filename)
#	return
#  
#  
#def createWRLPath():
#  filename = Blender.Get('filename')
#  if filename.find('.') != -1:
#    filename = filename.split('.')[0]
#    filename += '.' + fileExtension
#  return filename
#
#Blender.Window.FileSelector(select_file,"Export myRaw",createWRLPath())


def main():
	Blender.Window.FileSelector(exportToFile, 'JSON Export', Blender.sys.makename(ext='.json'))

if __name__=='__main__':
	main()
