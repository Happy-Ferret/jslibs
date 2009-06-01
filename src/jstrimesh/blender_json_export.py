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


	Write( '({' );
	for o in selectedObjectList:
	
		if o.getType() == "Empty":
			Write( o.name+':{type:"'+o.getType()+'",parent:"'+str(o.parent)+'",position:[%s,%s,%s]' % o.loc + '},' );
		
		if o.getType() == "Mesh":
			Write( o.name+':{type:"'+o.getType()+'",parent:"'+str(o.parent)+'",position:[%s,%s,%s]' % o.loc );


			BB = o.getBoundBox()
			objectCenter=[BB[0][n]+(BB[-2][n]-BB[0][n])/2.0 for n in [0,1,2]] 

			Write( ',center:[%s,%s,%s]' % (objectCenter[0],objectCenter[1],objectCenter[2]) );
			Write( ',size:[%s,%s,%s]' % o.size );
#do not need length prop.			Write( ',vertex:{ length:' + str(len(o.data.verts)) );
			Write( ',vertex:{');
			# format: x, y, z, normal.x, normal.y, normal.z, u, v
			for v in o.data.verts:
				Write('%s:[%s,%s,%s,%s,%s,%s,%s,%s,0],' % (v.index, v.co[0],v.co[1],v.co[2],v.no[0],v.no[1],v.no[2],v.uvco[0],v.uvco[1]))
			Write( '}' );

#
#			faceCount = 0
#			for f in o.data.faces:
#				if len(f.v) == 4:
#					faceCount += 2
#				else:
#					faceCount += 1
#			WriteLn( ',face:{ length:' + str(faceCount) );
#

			Write( ',face:[' );
			for f in o.data.faces:
				faceData = '%s,%s,%s,%s,%s' % (f.no[0],f.no[1],f.no[2],f.materialIndex,f.smooth )

				if len(f.v) == 4:
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[1].index, f.v[2].index) + ',' + faceData +']' )
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[2].index, f.v[3].index) + ',' + faceData +']' )
				else:
					Write(',[%s,%s,%s,' % (f.v[0].index, f.v[1].index, f.v[2].index) + ' ' + faceData +']' )
			Write( ']' );


			#material: http://www.blender.org/modules/documentation/237PythonDoc/Material.Material-class.html#specTransp
			
			materialCount = len(o.data.materials)
#			WriteLn('#materialCount')
#			WriteLn( str(materialCount) )

			Write(',material:[')
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

			Write( ']' );
			Write('},');
		
	Write( '})' );

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
