# Include all necessary modules here
# Client app needs only to include ONLY this file 
import sys
import os

# find project root directory from THIS file
ProjectRootDir = os.path.abspath(__file__ + '/../../') 
sys.path.append(ProjectRootDir + '/Core/Frontend/')

from FrontendAPI import *