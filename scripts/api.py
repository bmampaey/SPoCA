import subprocess
import tempfile
import os
import shutil
import glob

class SPoCAError(Exception):
	pass
class ClassificationError(SPoCAError):
	pass
class OverlayError(SPoCAError):
	pass

def build_arguments(table, bin, parameters, fitsfiles):
	arguments = [bin]

	for (k, v) in parameters.items():
		(flag, function) = table[k]
		arguments.append("-"+flag)
		if function is not None:
			arguments.append(function(v))
	
	arguments.extend(fitsfiles)

	return arguments
	

classification_parameters = {
	"centers_input_file": ("B", os.path.abspath),
	"number_classes": ("C", str),
	"intensities_stats_preprocessing": ("G", ",".join),
	"histogram_input_file": ("H", os.path.abspath),
	"image_type": ("I", str),
	"max_limits_file": ("L", os.path.abspath),
	"maps": ("M", ",".join),
	"neighbourhood_radius": ("N", str),
	"output_directory": ("O", os.path.abspath),
	"preprocessing": ("P", ",".join),
	"intensities_stats_radiusratio": ("R", str),
	"segmentation": ("S", str),
	"classifier": ("T", str),
	"chaincode_max_deviation": ("X", str),
	"classes_ar": ("a", lambda l: ",".join(map(str, l))),
	"classes_ch": ("c", lambda l: ",".join(map(str, l))),
	"classes_qs": ("q", lambda l: ",".join(map(str, l))),
	"fuzzifier": ("f", str),
	"max_iterations": ("i", str),
	"precision": ("p", str),
	"radiusratio": ("r", str),
	"thresholds": ("t", lambda l: ",".join(map(str, l))),
	"uncompressed": ("u", None),
	"chaincode_max_points": ("x", str),
	"bin_size": ("z", lambda l: ",".join(map(str, l)))
}



def classification(*fitsfiles, **parameters):
	root = os.path.abspath(".")
	
	if(len(fitsfiles) > 2):
		raise ClassificationError("Number of channels greater than 2 is not supported.")
	elif len(fitsfiles) == 2:
		bin = os.path.join(root, "bin2/classification.x")
	else:
		bin = os.path.join(root, "bin1/classification.x")
	
	if "output_directory" in parameters.keys():
		output_directory = parameters["output_directory"]
	else:
		output_directory = os.path.join(root, "results")
	
	parameters["output_directory"] = tempfile.mkdtemp()
	
	arguments = build_arguments(classification_parameters, bin, parameters, fitsfiles)
	print arguments
	try:
		p = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(out, err) = p.communicate()
		print out
		
		if err != "":
			raise ClassificationError(err)
		
		result_files = glob.glob(os.path.join(parameters["output_directory"], "*"))
		for result_file in result_files:
			shutil.copy(result_file, output_directory)
	finally:
		shutil.rmtree(parameters["output_directory"])
	
	return map(lambda f: os.path.join(output_directory, os.path.basename(f)), result_files)

overlay_parameters = {
	"colors_file": ("C", os.path.abspath),
	"label": ("L", str),
	"map": ("M", os.path.abspath),
	"output_directory": ("O", os.path.abspath),
	"preprocessing": ("P", ",".join),
	"size": ("S", lambda (width, height): str(width)+"x"+str(height)),
	"colors": ("c", lambda l: ",".join(map(str, l))),
	"external": ("e", None),
	"internal": ("i", None),
	"label": ("l", None),
	"mastic": ("m", None),
	"width": ("w", str)
}

def overlay(*fitsfiles, **parameters):
	root = os.path.abspath(".")
	
	if(len(fitsfiles) > 2):
		raise ClassificationError("Number of channels greater than 2 is not supported.")
	elif len(fitsfiles) == 2:
		bin = os.path.join(root, "bin2/overlay.x")
	else:
		bin = os.path.join(root, "bin1/overlay.x")
	
	if "output_directory" in parameters.keys():
		output_directory = parameters["output_directory"]
	else:
		output_directory = os.path.join(root, "results")
	
	parameters["output_directory"] = tempfile.mkdtemp()
	
	arguments = build_arguments(overlay_parameters, bin, parameters, fitsfiles)
	print arguments
	try:
		p = subprocess.Popen(arguments, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
		(out, err) = p.communicate()
		print out
		
		if err != "":
			raise ClassificationError(err)
		
		result_files = glob.glob(os.path.join(parameters["output_directory"], "*"))
		for result_file in result_files:
			shutil.copy(result_file, output_directory)
	finally:
		shutil.rmtree(parameters["output_directory"])
	
	return map(lambda f: os.path.join(output_directory, os.path.basename(f)), result_files)
