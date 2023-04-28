#include "../include/fileSystem.h"
#define PRIMER_BLOQUE_SECUNDARIO 1
#define LEER_DESDE_EL_INICIO 0

t_config* superbloque;
//FILE* bitmap;
void* bitmap;
void* bitmap_pointer;
t_bitarray* bitarray_de_bitmap;
FILE* bloques;
//t_config* directorio_FCB;
int tamanioBitmap;

t_log* logger;
t_config* config;
t_fileSystem_config lectura_de_config;

int kernel;

int main(int argc, char** argv) {

	//LECTURA DE CONFIG DEL FILESYSTEM
	logger = iniciar_logger("FileSystem.log", "FS");
	config = iniciar_config("../fileSystem.config");

    lectura_de_config = leer_fileSystem_config(config);

	//CHQUEO DE QUE LOS PATHS A LOS DIFERENTES ARCHIVOS EXISTEN(SUPERBLOQUE, DIRECTORIO_FCB, BITMAP, BLOQUES)

	//SUPERBLOQUE
	if (archivo_se_puede_leer(lectura_de_config.PATH_SUPERBLOQUE)){
		superbloque = iniciar_config(lectura_de_config.PATH_SUPERBLOQUE);
		super_bloque_info.block_size = config_get_int_value(superbloque, "BLOCK_SIZE");
		super_bloque_info.block_count = config_get_int_value(superbloque, "BLOCK_COUNT");
		log_info(logger, "SuperBloque leido");
	} else {
		log_error(logger, "SuperBloque no esiste");
		return EXIT_FAILURE;
	}

	//BITMAP
	int fd = open(lectura_de_config.PATH_BITMAP, O_CREAT | O_RDWR, 0664);	//abre o crea el archivo en un file descriptor
	if (fd == -1) {
		close(fd);
		log_error(logger, "Error abriendo el bitmap");
		return EXIT_FAILURE;
	}
	double c = (double) super_bloque_info.block_count;
	tamanioBitmap = (int) ceil( c/8.0 ); 	// tener en cuenta si no se necesita en otro lado
	bitmap_pointer = mmap(NULL, tamanioBitmap, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	bitarray_de_bitmap = bitarray_create_with_mode((char*) bitmap_pointer, tamanioBitmap, MSB_FIRST);
	close(fd);
	log_info(logger, "Bitmap abierto");

	//BLOQUES
	bloques = fopen(lectura_de_config.PATH_BLOQUES, "r+");
	if (bloques){
		log_info(logger, "Bloques existe");

	} else {
		return EXIT_FAILURE;
		bloques = fopen(lectura_de_config.PATH_BLOQUES, "w+");
	}

	//limpiar_bitmap();
	printf("unos inicial: %d\n", cant_unos_en_bitmap());

	//SE CONECTA AL SERIVDOR MEMORIA

	int socket_memoria = crear_conexion(lectura_de_config.IP_MEMORIA, lectura_de_config.PUERTO_MEMORIA);
    enviar_handshake(socket_memoria, FILESYSTEM);

	//SE HACE SERVIDOR Y ESPERA LA CONEXION DEL KERNEL
	//TODO CREAR UN HILO PARA ESPERAR EL CLIENTE(KERNEL)

	int server = iniciar_servidor("127.0.0.1", lectura_de_config.PUERTO_ESCUCHA);
	printf("Servidor listo para recibir al cliente: %d\n", server);
	kernel = esperar_cliente(server);
	if(kernel == -1){
		return -1;
	}

	puts("Se conecto el Kernel a FileSystem");

	while(1){
		t_instrucciones cod_op = recibir_cod_op(kernel);
		char* nombre_archivo;
		int nuevo_tamanio_archivo;
		int apartir_de_donde_X;
		int cuanto_X;
		int dir_fisica_memoria;
		char* buffer;

		switch (cod_op) {
			case ABRIR:{
				printf("abrir\n");
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n",nombre_archivo);
				if (existe_archivo(nombre_archivo)) {	//existe FCB?
					//enviar_mensaje_kernel(kernel, "OK El archivo ya existe");
					printf("existe/abierto %s\n",nombre_archivo);
				} else {
					//enviar_mensaje_kernel(kernel, "ERROR El archivo NO existe");
					printf("no existe %s\n",nombre_archivo);
				}
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case CREAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s \n", nombre_archivo);
				printf("unos antes crear: %d\n", cant_unos_en_bitmap());
				crear_archivo(nombre_archivo);	//crear FCB y poner tamaño 0 y sin bloques asociados.
				//enviar_mensaje_kernel(kernel, "OK Archivo creado");
				printf("archivo creado: %s\n",nombre_archivo);
				printf("unos dsp crear: %d\n", cant_unos_en_bitmap());
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case TRUNCAR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("nuevo_tamanio_archivo: %d\n", nuevo_tamanio_archivo);
				printf("unos antes truncar: %d\n", cant_unos_en_bitmap());
				truncar(nombre_archivo, nuevo_tamanio_archivo);
				//enviar_mensaje_kernel(kernel, "OK Archivo truncado");
				printf("unos dsp truncar: %d\n", cant_unos_en_bitmap());
				printf("\n");
				free(nombre_archivo);
				break;
			}
			case LEER:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				buffer = leer_archivo(nombre_archivo, apartir_de_donde_X, cuanto_X);	//malloc se hace en leer_archivo
				//mandar_a_memoria(socket_memoria, ESCRIBIR, buffer, cuanto_X, dir_fisica_memoria);
				//enviar_mensaje_kernel(kernel, "OK Archivo leido");
				//free(buffer);
				free(nombre_archivo);
				break;
			}
			case ESCRIBIR:{
				recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				printf("nombre_archivo: %s\n", nombre_archivo);
				printf("apartir_de_donde_X: %d\n", apartir_de_donde_X);
				printf("cuanto_X: %d\n", cuanto_X);
				printf("dir_fisica_memoria: %d\n", dir_fisica_memoria);
				//buffer = leer_de_memoria(socket_memoria, LEER, cuanto_X, dir_fisica_memoria);	//malloc se hace en leer_de_memoria
				escribir_archivo(buffer, nombre_archivo, apartir_de_donde_X, cuanto_X);
				//enviar_mensaje_kernel(kernel, "OK Archivo escrito");
				//free(buffer);
				free(nombre_archivo);
				break;
			}
			case ERROR:
				//recibir_parametros(cod_op, &nombre_archivo, &nuevo_tamanio_archivo, &apartir_de_donde_X, &cuanto_X, &dir_fisica_memoria);
				break;
			default:
				break;
		}
		msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	}
	config_destroy(superbloque);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	munmap(bitmap_pointer, tamanioBitmap);
	fclose(bloques);
	return EXIT_SUCCESS;
}

void escribir_archivo(char* buffer, char* nombre_archivo, int apartir_de_donde_escribir, int cuanto_escribir) {	//llega del switch
	if(!hay_espacio_suficiente(cuanto_escribir)){ //chequea que pueda escribi todo!
		log_error(logger, "No hay espacio suficiente para escribir");
		exit(EXIT_FAILURE);
	}
	if (apartir_de_donde_escribir > tamanio_archivo) {		//failsafe - no escribir donde no hay
		return "ERROR, apartir_de_donde_leer es > al tamanio_archivo";
	}
	int cantidad_escrita = 0;

	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	free(path);

	if(apartir_de_donde_escribir + cuanto_escribir > tamanio_archivo){
		int nuevo_tamanio_archivo = apartir_de_donde_escribir + cuanto_escribir;

		truncar_archivo(nombre_archivo, nuevo_tamanio_archivo);
	}

	if (apartir_de_donde_escribir < super_bloque_info.block_size){
		uint32_t puntero_directo = config_get_int_value(archivo_FCB, "PUNTERO_DIRECTO");
		fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
		int dist_fondo_bloque_apartir_de_donde_esc = super_bloque_info.block_size - apartir_de_donde_escribir;

		if(cuanto_escribir < dist_fondo_bloque_apartir_de_donde_esc){
			escribir_bloque(buffer, puntero_directo, apartir_de_donde_escribir, &cuanto_escribir, &cantidad_escrita);
		}else{
			escribir_bloque(buffer, puntero_directo, apartir_de_donde_escribir, &cuanto_escribir, &cantidad_escrita);				
			sobreescribir_indirecto(&buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, &cuanto_escribir, &cantidad_escrita);
		}
	}
	else {
		int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_escribir / super_bloque_info.block_size - 1);
		int apartir_de_donde_escribir_relativo_a_bloque = apartir_de_donde_escribir % super_bloque_info.block_size;
		escribir_bloque(buffer, bloque_secundario_inicial, apartir_de_donde_escribir_relativo_a_bloque, &cuanto_escribir, &cantidad_escrita);
		sobreescribir_indirecto(&buffer, archivo_FCB, bloque_secundario_inicial + 1, &cuanto_escribir, &cantidad_escrita);
	}
	
	config_destroy(archivo_FCB);
	return;
}

void sobreescribir_indirecto(char* buffer, t_config* archivo_FCB, int bloque_secundario_donde_escribir, int* cuanto_escribir, int* cantidad_escrita) {
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	uint32_t puntero_indirecto = config_get_int_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;			

	while(cuanto_escribir < 0) {
		fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * bloque_secundario_donde_escribir, SEEK_SET);
		fread(&puntero_secundario, sizeof(puntero_secundario), 1, bloques);	
		escribir_bloque(buffer, puntero_secundario, 0, &cuanto_escribir, &cantidad_escrita, super_bloque_info.block_size);
		bloque_secundario_donde_escribir++;
	}
}

void escribir_bloque(char* buffer, uint32_t puntero, int apartir_de_donde_escribir_relativo_a_puntero, int* cuanto_escribir, int* cantidad_escrita) {
	fseek(bloques, puntero * super_bloque_info.block_size + apartir_de_donde_escribir_relativo_a_puntero, SEEK_SET);

	char* copia_buffer = (char*) string_substring(buffer, *cantidad_escrita, *cuanto_escribir);

	if(apartir_de_donde_escribir_relativo_a_puntero + *cuanto_escribir > super_bloque_info.block_size){
		int dist_fondo_bloque_apartir_de_donde_esc = super_bloque_info.block_size - apartir_de_donde_escribir_relativo_a_bloque;
		fwrite(copia_buffer, dist_fondo_bloque_apartir_de_donde_esc, 1, bloques);

		*cantidad_escrita += dist_fondo_bloque_apartir_de_donde_esc;
		*cuanto_escribir -= dist_fondo_bloque_apartir_de_donde_esc;
	}
	else {
		fwrite(copia_buffer, *cuanto_escribir, 1, bloques);
		*cantidad_escrita += *cuanto_escribir;
		*cuanto_escribir = 0;
	}
}

void truncar(char* nombre_archivo, int nuevo_tamanio_archivo){
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);

	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	if (nuevo_tamanio_archivo > tamanio_archivo){
		agrandas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}
	else if (nuevo_tamanio_archivo < tamanio_archivo) {
		achicas_archivo(archivo_FCB, nombre_archivo, nuevo_tamanio_archivo);
	}

	char* tam = string_itoa(nuevo_tamanio_archivo);
	config_set_value(archivo_FCB, "TAMANIO_ARCHIVO", tam);
	printf("tam = %s\n", tam);
	free(tam);
	config_save_in_file(archivo_FCB, path);
	config_destroy(archivo_FCB);
	free(path);
}

uint32_t config_get_uint_value(t_config *self, char *key) {
	char *value = config_get_string_value(self, key);
	char *ptr;
	return (uint32_t) strtoul(value, &ptr, 10);
}

void achicas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//entro si tiene puntero_directo y ya tiene puntero_indirecto
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) (nuevo_tamanio_archivo ? nuevo_tamanio_archivo : 1 ) / super_bloque_info.block_size - 1);
		if(cant_punteros_secundarios > cant_bloques_secundarios_necesarios){
			for( ; cant_punteros_secundarios > cant_bloques_secundarios_necesarios ; cant_punteros_secundarios--) {
				uint32_t puntero;
				fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * (cant_punteros_secundarios-1), SEEK_SET);
				fread(&puntero, sizeof(puntero), 1, bloques);
				liberar_bloque(puntero);
			}
		}
		if(nuevo_tamanio_archivo <= super_bloque_info.block_size){
			liberar_bloque(puntero_indirecto);
			config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", "");
		}
	}
	if(nuevo_tamanio_archivo == 0){
		uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
		liberar_bloque(puntero_directo);
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", "");
	}
	return;
}

void agrandas_archivo(t_config* archivo_FCB, char* nombre_archivo, int nuevo_tamanio_archivo){
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");

	//CADENA DE IFs, va entrando si cumple condicion
	//entro si no tiene puntero_directo
	if(tamanio_archivo == 0){
		uint32_t puntero_directo = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_DIRECTO", string_itoa(puntero_directo));
	}
	//entro si tiene puntero_directo, no tiene puntero_indirecto y si el nuevo tamanio requiere de punteros_secundarios
	if(tamanio_archivo <= super_bloque_info.block_size && nuevo_tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = dame_un_bloque_libre();
		config_set_value(archivo_FCB, "PUNTERO_INDIRECTO", string_itoa(puntero_indirecto));

		int cant_punteros_secundarios = 0;
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
			fwrite(&puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
		}
	}
	//entro si tiene puntero_directo, tiene puntero_indirecto (tiene 1 o + punteros_secundarios)
	if(tamanio_archivo > super_bloque_info.block_size){
		uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");

		int cant_punteros_secundarios = (int) ceil((float) tamanio_archivo / super_bloque_info.block_size - 1);
		int cant_bloques_secundarios_necesarios = (int) ceil((float) nuevo_tamanio_archivo / super_bloque_info.block_size - 1);
		for( ; cant_punteros_secundarios < cant_bloques_secundarios_necesarios ; cant_punteros_secundarios++) {
			uint32_t puntero = dame_un_bloque_libre();
			fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero) * cant_punteros_secundarios, SEEK_SET);
			fwrite(&puntero, sizeof(puntero), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto
		}
	}
	return;
}

void crear_archivo(char* nombre_archivo) {	//necesita semaforos para hilos pq entra a archivo comun FCBdefault
	char* path = obtener_path_FCB_sin_free(nombre_archivo);

	t_config* FCBdefault = iniciar_config("../FCBdefault");
	config_set_value(FCBdefault, "NOMBRE_ARCHIVO", nombre_archivo);
	config_set_value(FCBdefault, "TAMANIO_ARCHIVO", "0");
	config_set_value(FCBdefault, "PUNTERO_DIRECTO", "");
	config_set_value(FCBdefault, "PUNTERO_INDIRECTO", "");
	config_save_in_file(FCBdefault, path);
	config_destroy(FCBdefault);					//cierro el FCB
	free(path);
}

bool existe_archivo(char* nombre_archivo) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	bool retorno = archivo_se_puede_leer(path);
	free(path);
	return retorno;
}

char* obtener_path_FCB_sin_free(char* nombre_archivo){
	char* path = malloc(strlen(lectura_de_config.PATH_FCB) + strlen(nombre_archivo));
	strcpy(path, lectura_de_config.PATH_FCB);
	strcat(path, nombre_archivo);		//se asume que en la FS.config el path_fcb tiene / al final quedando: "/home/utnso/fs/fcb/"
	return path;
}

char* leer_archivo(char* nombre_archivo, int apartir_de_donde_leer, int cuanto_leer) {
	char* path = obtener_path_FCB_sin_free(nombre_archivo);
	t_config* archivo_FCB = iniciar_config(path);
	free(path);

	//FAIL CHECKERS
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	if (apartir_de_donde_leer > tamanio_archivo) {
		return "ERROR, apartir_de_donde_leer es > al tamanio_archivo";
	}
	if (apartir_de_donde_leer + cuanto_leer > tamanio_archivo) {
		cuanto_leer = tamanio_archivo - apartir_de_donde_leer;		//deberia tirar error?
		//return "ERROR, apartir_de_donde_leer + cuanto_leer es > al tamanio_archivo";
	}

	//LECTURA
	char* buffer = malloc(cuanto_leer);
	if (tamanio_archivo != 0) {
		//entro si leo de puntero_directo
		if (apartir_de_donde_leer < super_bloque_info.block_size){
			uint32_t puntero_directo = config_get_uint_value(archivo_FCB, "PUNTERO_DIRECTO");
			fseek(bloques, puntero_directo * super_bloque_info.block_size, SEEK_SET);
			int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer;

			if(cuanto_leer < cant_disponible_leer_de_puntero){
				fread(buffer, cuanto_leer, 1, bloques);
				cuanto_leer = 0;
			}else{
				fread(buffer, cant_disponible_leer_de_puntero, 1, bloques);
				cuanto_leer -= cant_disponible_leer_de_puntero;
				leer_indirecto(&buffer, archivo_FCB, PRIMER_BLOQUE_SECUNDARIO, LEER_DESDE_EL_INICIO, cuanto_leer);
			}
		}
		//entro si leo de puntero indirecto y cualquier puntero secundario
		if (apartir_de_donde_leer >= super_bloque_info.block_size){
			int bloque_secundario_inicial = (int) ceil((float) apartir_de_donde_leer / super_bloque_info.block_size - 1);
			int apartir_de_donde_leer_relativo_a_bloque = apartir_de_donde_leer%super_bloque_info.block_size;
			leer_indirecto(&buffer, archivo_FCB, bloque_secundario_inicial, apartir_de_donde_leer_relativo_a_bloque, cuanto_leer);
		}
	}
	else {
		//logear error
		//"No puedo leer archivo vacio"?
		config_destroy(archivo_FCB);
		exit(EXIT_FAILURE);
	}
	config_destroy(archivo_FCB);
	return buffer;	//el free se hace en el switch, despues de pasarle el contenido a memoria
}

void leer_indirecto(char** buffer, t_config* archivo_FCB, int bloque_secundario_donde_leer, int apartir_de_donde_leer_relativo_a_bloque, int cuanto_leer) {
	int tamanio_archivo = config_get_int_value(archivo_FCB, "TAMANIO_ARCHIVO");
	uint32_t puntero_indirecto = config_get_uint_value(archivo_FCB, "PUNTERO_INDIRECTO");
	uint32_t puntero_secundario;
	fseek(bloques, puntero_indirecto * super_bloque_info.block_size + sizeof(puntero_secundario) * bloque_secundario_donde_leer, SEEK_SET);
	fread(&bloque_secundario_donde_leer, sizeof(puntero_secundario), 1, bloques);				//anoto nuevo puntero_secundario en bloque de puntero_indirecto

	int cant_disponible_leer_de_puntero = super_bloque_info.block_size - apartir_de_donde_leer_relativo_a_bloque;
	if(cuanto_leer < cant_disponible_leer_de_puntero){
		char * buffer1 = malloc(cuanto_leer);
		fread(buffer1, cuanto_leer, 1, bloques);
		cuanto_leer = 0;
		strcat(*buffer, buffer1);
		free(buffer1);
		strcat(*buffer, buffer1);
	}else{
		char * buffer1 = malloc(cant_disponible_leer_de_puntero);
		fread(buffer1, cant_disponible_leer_de_puntero, 1, bloques);
		cuanto_leer -= cant_disponible_leer_de_puntero;
		strcat(*buffer, buffer1);
		free(buffer1);
		leer_indirecto(&buffer, archivo_FCB, bloque_secundario_donde_leer+1, LEER_DESDE_EL_INICIO, cuanto_leer);
	}
}

bool archivo_se_puede_leer(char* path)
{
	FILE* f;
	if(f = fopen(path, "r")){
		fclose(f);
		return 1;
	} else {
		return 0;
	}
}

bool checkear_espacio(int cuanto_escribir) {
	int bloques_necesarios = cuanto_escribir / super_bloque_info.block_size;
	if (cuanto_escribir % super_bloque_info.block_size != 0)
		bloques_necesarios++;
	if (bloques_necesarios > bitarray_get_max_bit(bitarray_de_bitmap))
		return false;
	return true;
}

uint32_t dame_un_bloque_libre() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 0) {
			bitarray_set_bit(bitarray_de_bitmap, i);
			msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
			return i;
		}
	}
	return -1;	
}

int cant_unos_en_bitmap(){
	int contador = 0;
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		if (bitarray_test_bit(bitarray_de_bitmap, i) == 1) {
			contador++;
		}
	}
	return contador;
}

void limpiar_bitmap() {
	for (int i = 0; i < bitarray_get_max_bit(bitarray_de_bitmap); i++) {
		bitarray_clean_bit(bitarray_de_bitmap, i);
	}
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
	return;
}

void liberar_bloque(uint32_t puntero){
	bitarray_clean_bit(bitarray_de_bitmap, puntero);
	msync(bitmap_pointer, tamanioBitmap, MS_SYNC);
}

t_instrucciones recibir_cod_op(int socket_cliente)
{
	t_instrucciones cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(t_instrucciones), 0x100) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return ERROR;
	}
}

void recibir_parametros(t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	/*F_OPEN ARCHIVO
	F_TRUNCATE ARCHIVO 64
	F_WRITE ARCHIVO 4 4
	F_READ ARCHIVO 16 4*/
	size_t largo_nombre;

	size_t size_payload;
	if (recv(kernel, &size_payload, sizeof(size_t), 0) != sizeof(size_t)){
		return;
	}

	void* a_recibir = malloc(size_payload);
	if (recv(kernel, a_recibir, size_payload, 0) != size_payload) {
		free(a_recibir);
		return;
	}

	deserializar_instrucciones_kernel(a_recibir, size_payload, cod_op, nombre_archivo, tamanio_nuevo_archivo, apartir_de_donde_X, cuanto_X, dir_fisica_memoria);

	free(a_recibir);
	return;
}

void deserializar_instrucciones_kernel(void* a_recibir, int size_payload, t_instrucciones cod_op, char** nombre_archivo, int* tamanio_nuevo_archivo, int* apartir_de_donde_X, int* cuanto_X, int* dir_fisica_memoria){
	switch(cod_op){
		case ABRIR:
		case CREAR:
		default:
			int desplazamiento = 0;
			size_t largo_nombre;
			memcpy(&largo_nombre, a_recibir + desplazamiento, sizeof(largo_nombre));
			desplazamiento += sizeof(largo_nombre);
			//printf("largo_nombre: %d\n", largo_nombre);
			char* aux_nombre_file = malloc(largo_nombre);
			memcpy(aux_nombre_file, a_recibir + desplazamiento, largo_nombre);
			desplazamiento += largo_nombre;
			//printf("nombre_archivo: %s\n", aux_nombre_file);
			*nombre_archivo = strdup(aux_nombre_file);
			switch(cod_op){
				case TRUNCAR:
					memcpy(tamanio_nuevo_archivo, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
				case LEER:
				case ESCRIBIR:
					memcpy(apartir_de_donde_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					memcpy(cuanto_X, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					memcpy(dir_fisica_memoria, a_recibir + desplazamiento, sizeof(int));
					desplazamiento += sizeof(int);
					break;
			}
			break;
		case ERROR:
			break;
		}
}

void enviar_mensaje_kernel(int kernel, char* msj){
	return;
}
char* leer_de_memoria(int socket_memoria, t_instrucciones LEER, int cuanto_escribir, int dir_fisica_memoria){
	return "hey";
}
void mandar_a_memoria(int socket_memoria, t_instrucciones ESCRIBIR, char* buffer, int cuanto_leer, int dir_fisica_memoria){
	return;
}







