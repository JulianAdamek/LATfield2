/*! file IOserver.cpp
    Created by David Daverio.
 
    A simple example of LATfield2d usage. This exec solve the poisson equation in fourier space.
    
 
 */



#include <iostream>
#include "LATfield2.hpp"

using namespace LATfield2;



int main(int argc, char **argv)
{
    int n,m;
    int io_size;
    int io_groupe_size;
    
    
    for (int i=1 ; i < argc ; i++ ){
		if ( argv[i][0] != '-' )
			continue;
		switch(argv[i][1]) {
			case 'n':
				n = atoi(argv[++i]); 
				break;
			case 'm':
				m =  atoi(argv[++i]);
				break;
            case 'i':
                io_size =  atoi(argv[++i]);
                break;
            case 'g':
                io_groupe_size = atoi(argv[++i]);
                break;
		}
	}
	parallel.initialize(n,m,io_size,io_groupe_size);
    
    /*!
     Part of the code which will be executed by the IOserver process. In fact this cores should only start the server
     */
    if(parallel.isIO()) IO_Server.start();
    else
    {
        /*!
         Part of the code which will be executed by the compute process.
        
         For this example each processor will write 100 times the same line within one file with the following path: "./testfile"
         */
        
        /*!
         Create line: I am the "rank_world" MPI proces. My rank is "rank" in the compute group. I have the position ("N","M") in the process grid. 
         */
        
        
        
        string sentence;
        sentence = "I am the " + int2string(parallel.world_rank(),99999) + " MPI process. My rank is ";
        sentence += int2string(parallel.rank(),99999) + " in the compute group. I have the position (";
        sentence += int2string(parallel.grid_rank()[0],99999) + "," + int2string(parallel.grid_rank()[1],99999);
        sentence += ") in the processes grid." ;
        
        char * sendbuffer;
        sendbuffer = (char*)malloc(sentence.size()+1);
        for(int i=0;i<sentence.size();i++)sendbuffer[i]=sentence[i];
        sendbuffer[sentence.size()]='\n'; 
        
        int nparts=100;
        part_simple pcls[nparts];
        part_simple_dataType pcls_types;
        
        for(int i = 0;i<nparts;i++)
        {
            pcls[i].ID=i;
            pcls[i].pos[0]=1.1;
            pcls[i].pos[1]=2.1;
            pcls[i].pos[2]=3.1;
            pcls[i].vel[0]=1.1;
            pcls[i].vel[1]=2.2;
            pcls[i].vel[2]=3.3;
        }
        
        IO_Server.openOstream();
        
        
        Lattice lat(3,64,2);
        Field<float> phi(lat,3);
        
        Site x(lat);
        
        for(x.first();x.test();x.next())
        {
            phi(x,0)=x.coord(0);
            phi(x,1)=x.coord(1);
            phi(x,2)=x.coord(2);
            
        }
        
        phi.saveHDF5_server_open("testPhi",10,10);
        phi.saveHDF5_server_write(5);
        
        ioserver_file ubin_file,ubin_file1,uh5_file,sh5_file;
        
        hid_t datatype = H5T_NATIVE_FLOAT;
        
        ubin_file = IO_Server.openFile("test_ubin",UNSTRUCTURED_BIN_FILE);
        uh5_file = IO_Server.openFile("test_uh5",UNSTRUCTURED_H5_FILE,pcls_types.part_memType,pcls_types.part_fileType);
        //sh5_file = IO_Server.openFile("test_sh5",STRUCTURED_H5_FILE,datatype,datatype,&lat,3,1);
        
        //sleep(1);
       
        
        hsize_t size[lat.dim()];
        hsize_t offset[lat.dim()];
        
        
        for(int i=0;i<lat.dim();i++)
        {
            size[i] = lat.sizeLocal(i);
        }
        offset[0]=0;
        offset[1]=lat.coordSkip()[1];
        offset[2]=lat.coordSkip()[0];
        
        //IO_Server.sendData(sh5_file,(char*)phi.data(),size,offset);
        
        IO_Server.sendData(ubin_file,sendbuffer,sentence.size()+1);
        IO_Server.sendData(ubin_file,sendbuffer,sentence.size()+1);
        
        IO_Server.sendData(uh5_file,(char*)pcls,nparts*sizeof(part_simple));
        
        
        float testAttr[4] = {31.3,2.3,4.4,5};
        datatype =H5T_NATIVE_FLOAT;
        string attr_name = "test";
        IO_Server.sendATTR(uh5_file,attr_name,(char*)&testAttr,1,datatype);
        attr_name = "test3";
        IO_Server.sendATTR(uh5_file,attr_name,(char*)&testAttr,3,datatype);
        //IO_Server.sendData(sh5_file,(char*)phi.data(),size,offset);
        
        
    
        
        hsize_t dset_dim = 3;
        hsize_t dset_size[3] = {4,4,2};
        float dset[dset_size[0]*dset_size[1]*dset_size[2]];
        
        for(int i=0;i<dset_size[0]*dset_size[1]*dset_size[2];i++)dset[i]=0.1+i;
        
        IO_Server.sendDataset(uh5_file,"blabla",(char *)dset, dset_dim, dset_size, H5T_NATIVE_FLOAT);
        
        
        
        IO_Server.closeFile(ubin_file);
        IO_Server.closeFile(uh5_file);
        //IO_Server.closeFile(sh5_file);
        
        IO_Server.closeOstream();
        
        //if(IO_Server.openOstream()== OSTREAM_FAIL) cout<< "arg"<<endl ;
        
        //while(IO_Server.openOstream()== OSTREAM_FAIL){}
        
        
        //IO_Server.closeOstream();
        
        IO_Server.stop();
    }
    
    
    
    
}