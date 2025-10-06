/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "winebox.h"
#include "platform.h"
#include "kelf.h"
#include "kmmap.h"
#include "kthread.h"
#include "kprocess.h"
#include "fsapi.h"

#include <string.h>

#include UNISTD

#define SHF_WRITE 1
#define SHF_ALLOC 2

#define SHT_NOBITS 8

#define PT_LOAD 1
#define PT_INTERP 3 

static char interp[MAX_FILEPATH_LEN];
static char shell_interp[MAX_FILEPATH_LEN];

char* getInterpreter(struct FsOpenNode* openNode, BOOL* isElf) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)];
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U32 len = openNode->func->readNative(openNode, buffer, sizeof(buffer));

    *isElf=TRUE;
    if (len!=sizeof(buffer)) {
        *isElf=FALSE;
    }
    if (*isElf) {		
        *isElf = isValidElf(hdr);
    }
    if (!*isElf) {
        if (buffer[0]=='#') {
            U32 i;
            U32 mode = 0;
            U32 pos = 0;

            for (i=1;i<len;i++) {
                if (mode==0) {
                    if (buffer[i]=='!')
                        mode = 1;
                } else if (mode==1 && (buffer[i]==' ' || buffer[i]=='\t')) {
                    continue;
                } else if (buffer[i]=='\n' || buffer[i]=='\r') {
                    break;
                } else {
                    mode = 2;
                    shell_interp[pos++] = buffer[i];
                }
            }
            shell_interp[pos++]=0;
            return shell_interp;
        }
    } else {
        U32 i;

        openNode->func->seek(openNode, hdr->e_phoff);	
        for (i=0;i<hdr->e_phoff;i++) {
            struct k_Elf32_Phdr phdr;		
            openNode->func->seek(openNode, hdr->e_phoff+i*hdr->e_phentsize);	
            openNode->func->readNative(openNode, (U8*)&phdr, sizeof(struct k_Elf32_Phdr));
            if (phdr.p_type==PT_INTERP) {
                openNode->func->seek(openNode, phdr.p_offset);	
                openNode->func->readNative(openNode, (U8*)interp, phdr.p_filesz);
                interp[phdr.p_filesz] = 0;
                return interp;
            }
        }
    }
    return 0;
}

BOOL inspectNode(struct KProcess* process, const char* currentDirectory, struct FsNode* node, const char** loader, const char** interpreter, const char** interpreterArgs, U32* interpreterArgsCount, struct FsOpenNode** result) {
    BOOL isElf = 0;
    struct FsOpenNode* openNode = 0;
    struct FsNode* interpreterNode = 0;
    struct FsNode* loaderNode = 0;

    if (node) {
        openNode = node->func->open(process, node, K_O_RDONLY);
    }
    if (openNode) {
        char* arg;

        *interpreterArgsCount=0;
        *interpreter = getInterpreter(openNode, &isElf);
        if (isElf) {
            *loader = *interpreter;
            *interpreter = 0;
        } else if (*interpreter) {
            arg=(char*)*interpreter;
            while ((arg=strchr(arg, ' '))) {
                arg[*interpreterArgsCount]=0;
                arg++;
                interpreterArgs[*interpreterArgsCount]=arg;
                *interpreterArgsCount=*interpreterArgsCount+1;
            }
        }
        openNode->func->close(openNode);
        openNode = 0;
    }
    if (!*interpreter && !isElf) {
        return FALSE;
    }
    if (*interpreter) {
        interpreterNode = getNodeFromLocalPath(currentDirectory, *interpreter, TRUE);	
        if (!interpreterNode || !interpreterNode->func->exists(interpreterNode)) {
            printf("Interpreter not found: %s\n", *interpreter);
            return FALSE;
        }
        openNode = interpreterNode->func->open(process, interpreterNode, K_O_RDONLY);		
        *loader = getInterpreter(openNode, &isElf);
        openNode->func->close(openNode);
        openNode = 0;
    }
    if (*loader) {
        loaderNode = getNodeFromLocalPath(currentDirectory, *loader, TRUE);	
        if (!loaderNode || !loaderNode->func->exists(loaderNode)) {
            return FALSE;
        }
    }		

    if (loaderNode) {
        *result = loaderNode->func->open(process, loaderNode, K_O_RDONLY);
    } else if (interpreterNode) {
        *result = interpreterNode->func->open(process, interpreterNode, K_O_RDONLY);		
    } else {
        *result = node->func->open(process, node, K_O_RDONLY);
    }
    if (*result)
        return TRUE;
    return FALSE;
}

int getMemSizeOfElf(struct FsOpenNode* openNode) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)];
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U32 len;
    U64 pos = openNode->func->getFilePointer(openNode);
    U32 address = 0xFFFFFFFF;
    int i;
    int sections = 0;

    openNode->func->seek(openNode, 0);
    len = openNode->func->readNative(openNode, buffer, sizeof(buffer));
    if (len != sizeof(buffer)) {
        return 0;
    }
    if (!isValidElf(hdr)) {
        openNode->func->seek(openNode, pos);
        return 0;
    }

    len = 0;
    openNode->func->seek(openNode, hdr->e_phoff);
    for (i = 0; i<hdr->e_phnum; i++) {
        struct k_Elf32_Phdr phdr;
        openNode->func->readNative(openNode, (U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type == PT_LOAD) {
            if (phdr.p_paddr<address)
                address = phdr.p_paddr;
            if (len<phdr.p_paddr + phdr.p_memsz)
                len = phdr.p_paddr + phdr.p_memsz;
            sections++;
        }
    }
    openNode->func->seek(openNode, pos);
    return len - address + 4096*sections; // 4096 for alignment
}
#if 0
U32 getPELoadAddress(struct FsOpenNode* openNode, U32* section, U32* numberOfSections, U32* sizeOfSection) {
    static U8 buffer[1024];	
    U64 pos = openNode->func->getFilePointer(openNode);
    U32 len;
    int offset;
    U32 sizeOfOptionalHeader;

    openNode->func->seek(openNode, 0);
    len = openNode->func->readNative(openNode, buffer, sizeof(buffer));
    openNode->func->seek(openNode, pos);
    if (len != sizeof(buffer)) {		
        return FALSE;
    }
    // DOS Magic MZ
    if (buffer[0] != 0x4D || buffer[1] != 0x5A) {
        return 0;
    }

    // offset is pointer to IMAGE_NT_HEADERS
    offset = buffer[0x3C] | ((int)buffer[0x3D] << 8);

    // check IMAGE_NT_HEADERS.Signature
    if (buffer[offset] != 0x50 || buffer[offset + 1] != 0x45 || buffer[offset + 2] != 0 || buffer[offset + 3] != 0) {
        return 0;
    }
    // IMAGE_NT_HEADERS.FileHeader.NumberOfSections
    *numberOfSections = buffer[offset + 0x6] | ((U32)buffer[offset + 0x7] << 8);

    *sizeOfSection = 0x28;

    // IMAGE_NT_HEADERS.FileHeader.SizeOfOptionalHeader
    sizeOfOptionalHeader = buffer[offset + 0x14] | ((U32)buffer[offset + 0x15] << 8);

    // section should not reference buffer, but be mapped into memory
    *section = (U32)(buffer + offset + 0x14 /*sizeof(IMAGE_FILE_HEADER)*/ + sizeOfOptionalHeader + 4 /*sizeof(IMAGE_NT_HEADERS.Signature)*/);

    // IMAGE_NT_HEADERS.OptionalHeader.ImageBase
    return buffer[offset + 0x34] | ((U32)buffer[offset + 0x35] << 8) | ((U32)buffer[offset + 0x36] << 16) | ((U32)buffer[offset + 0x37] << 24);
}
#endif
BOOL loadProgram(struct KProcess* process, struct KThread* thread, struct FsOpenNode* openNode, U32* eip) {
    U8 buffer[sizeof(struct k_Elf32_Ehdr)];
    struct k_Elf32_Ehdr* hdr = (struct k_Elf32_Ehdr*)buffer;
    U32 len = openNode->func->readNative(openNode, buffer, sizeof(buffer));
    U32 address=0xFFFFFFFF;
    U32 i;
    U32 reloc;

    if (len!=sizeof(buffer)) {
        return FALSE;
    }
    if (!isValidElf(hdr))
        return FALSE;    
    len=0;
    openNode->func->seek(openNode, hdr->e_phoff);	
    for (i=0;i<hdr->e_phnum;i++) {
        struct k_Elf32_Phdr phdr;		
        openNode->func->readNative(openNode, (U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type==PT_LOAD) {
            if (phdr.p_paddr<address) {
                address=phdr.p_paddr;
            }
            if (len<phdr.p_paddr+phdr.p_memsz)
                len=phdr.p_paddr+phdr.p_memsz;
        }
    }

    if (address>0x10000) {
        reloc = 0;
        len-=address;
    } else {
        reloc = ADDRESS_PROCESS_LOADER<<PAGE_SHIFT;
        address = reloc;
    }

    if (reloc)
        address = syscall_mmap64(thread, address, len, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0);
    process->loaderBaseAddress = address;
    process->brkEnd = address+len;
    process->phdr = 0;

    for (i=0;i<hdr->e_phnum;i++) {
        struct k_Elf32_Phdr phdr;		
        openNode->func->seek(openNode, hdr->e_phoff+hdr->e_phentsize*i);
        openNode->func->readNative(openNode, (U8*)&phdr, sizeof(struct k_Elf32_Phdr));
        if (phdr.p_type==PT_LOAD) {
            if (!reloc) {
                U32 addr = phdr.p_paddr;
                U32 len = phdr.p_memsz;

                if (phdr.p_paddr & 0xFFF) {
                    addr &= 0xFFFFF000;
                    len+=(phdr.p_memsz+(phdr.p_paddr-addr));
                }
                syscall_mmap64(thread, addr, len, K_PROT_READ | K_PROT_WRITE | K_PROT_EXEC, K_MAP_PRIVATE | K_MAP_ANONYMOUS | K_MAP_FIXED, -1, 0);
            }
            if (phdr.p_filesz>0) {
                if (phdr.p_offset<=hdr->e_phoff && hdr->e_phoff<phdr.p_offset+phdr.p_filesz) {
                    process->phdr = reloc+phdr.p_paddr+hdr->e_phoff-phdr.p_offset;
                }
                openNode->func->seek(openNode, phdr.p_offset);                
                openNode->func->read(thread, openNode, reloc+phdr.p_paddr, phdr.p_filesz);		
            }
        }
    }
    process->phentsize=hdr->e_phentsize;
    process->phnum=hdr->e_phnum;

    *eip = hdr->e_entry+reloc;
    process->entry = *eip; 
    return TRUE;
}
