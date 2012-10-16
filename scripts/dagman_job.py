import os, sys, tempfile

CONDOR_JOB_DESCRIPTION_TEMPLATE="""
####################
##
## Condor job description file created by the Condor.py module
##
####################


# Check the Condor primer on _which universe to choose
# (standard / vanilla)
Universe        = vanilla

# The absolute path to Executable
Executable      = {executable}

# Arguments for the executable
Arguments	= {arguments}

# Input/Output files
Input          =  {input_file}
Output          = {output_file}
Error           = {error_file}
Log             = {log_file}

# The filesytem is shared
requirements		= (HasPOOL =?= True)

# Parameters for condor
getenv = True
log_xml = True
should_transfer_files	= IF_NEEDED
when_to_transfer_output = ON_EXIT
transfer_executable	= NO

# Tell condor to run the job
queue

"""

class Job:
	def __init__(self, name, executable, arguments, require=[]):
		(jobfile, self.jobfilepath) = tempfile.mkstemp()
		os.write(jobfile, CONDOR_JOB_DESCRIPTION_TEMPLATE.format(executable=executable, arguments=arguments, input_file="/dev/null", output_file="/dev/null", error_file="/dev/null", log_file=self.jobfilepath+".log"))
		os.close(jobfile)

		self.require = require
		self.name = name

class DAG:
	def __init__(self, jobs):
		self.jobs = jobs
	def submit(self):
		(dagfile, dagfilepath) = tempfile.mkstemp()
		
		for job in self.jobs:
			os.write(dagfile, "JOB " + job.name + " " + job.jobfilepath + "\n")
		for job in self.jobs:
			if job.require != [] and job.require is not None:
				os.write(dagfile, "PARENT " + " ".join(map(lambda x: x.name, job.require)) + " CHILD " + job.name + "\n")
		os.close(dagfile)

		print dagfilepath
