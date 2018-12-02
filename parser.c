// Librerias 
#include "anlex.h"

// Variables
token t; 			// token global para recibir componentes del Analizador Lexico
FILE *archivo; 		// Fuente JSON
FILE *salida; 		// Traducción en XML
char trad[41]; 		// Traducción 
short pos_tab=0; 	// Posicion de la tabulacion.
char mensaje[41]; 		// Mensaje de error.
short bandera_error=0; // Bandera para mensajes.




//prototipos de funciones para cada NO TERMINAL
void json(void);
void element(void);
void array(void);
void array_prima(void);
void element_list(void);
void el_prima(void);
void object(void);
void o_prima(void);
void attribute_list(void);
void attribute(void);
void al_prima(void);
void attribute_name(void);
void attribute_value(void);

// Prototipos de funciones de traducción.
char* tabulador(char*);
void destabulador(char*);


//implementacion de funciones

void error_mensaje(char* mensaje){
    // Función de mensaje de error.
    bandera_error=1;
    printf("Lin %d:\t\nERROR: La sintaxis no es correcta. \n\t%s.\n",linea(),mensaje);	
    //exit(1);
}

// Función que concatena tabulaciones.
char* tabulador(char* texto){
    char *tabs= calloc(10000,sizeof(texto));
    for(short i=0;i<pos_tab;i++){
        strcat(tabs,"\t");
    }
    strcat(tabs,texto);
    return tabs;
}

// Remueve todas las tabulaciones.
void destabulador(char* texto){   
    char *pr = texto, *pw = texto;
    while (*pr) {
        *pw = *pr++;
        pw += (*pw != '\t');
    }
    *pw = '\0';
}

void getToken(void) {
    //retorna la siguiente entrada de la fuente
    sigLex();
}


void match(char* c){
    //verifica si el token actual es igual al token a comparar
    //y luego llama al siguiente token de la fuente
    if (strcmp(t.compLex, c)==0){
        getToken(); 
    }else
        error_mensaje("ERROR: error al hacer match.");
}

void json(){
    //se llama a la regla gramatical element
    element();

    //se llega al final del archivo
    match("EOF");
}

void element(){
    //element: regla de la gramatica

    if(strcmp(t.compLex,"L_LLAVE")==0){
        //si el token es una llave abierto, se espera que sea un objeto  
        object();
    }else if (strcmp(t.compLex,"L_CORCHETE")==0){
        //si el token es corchete abierto, se espera que sea una lista
        array();
    }else if(strcmp(t.compLex,"R_CORCHETE")==0 || strcmp(t.compLex,"R_LLAVE")==0||strcmp(t.compLex,"COMA")==0){
         //si el token es llave o corchete cerrada, o una coma, significa error
        sprintf(mensaje,"Se esperaba los siguientes tokens \"{\" o \"[\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }else 
        //si no es ninguno de lo anterior, se lee el siguiente token
        getToken();
}

void array(){
    if(strcmp(t.compLex,"L_CORCHETE")==0){
        //si el token es una corchete abierto, se hace match y se llama a array prima  
        match("L_CORCHETE");
        array_prima();  
        strcpy(trad,tabulador(trad));
        pos_tab--; // Cuando el array termina se baja un nivel.   
    }
    else if(strcmp(t.compLex,"R_CORCHETE")==0||strcmp(t.compLex,"R_LLAVE")==0||strcmp(t.compLex,"COMA")==0){
       //si el token actual es una llave o corchete cerrado, significa un error 
        sprintf(mensaje,"Se esperaba se esperaba los siguientes tokens \"[\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }
    else getToken();
}

void array_prima(){
    //si el token es un corchete cerrado
    if(strcmp(t.compLex,"R_CORCHETE")==0){
        match("R_CORCHETE"); //
    }else if(strcmp(t.compLex,"L_CORCHETE")==0||strcmp(t.compLex,"L_LLAVE")==0){
        //si el token es un corchete abierto
        element_list();
        match("R_CORCHETE");
    }else if(strcmp(t.compLex,"R_LLAVE")==0){
        //si el token es una llave cerrada
        sprintf(mensaje,"Se esperaba se esperaba los siguientes tokens \"[\" o \"]\" o \"{\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }else
        getToken();
}

void element_list(){
    //si el token es un corchete abierto o una llave abierta
    if(strcmp(t.compLex,"L_CORCHETE")==0||strcmp(t.compLex,"L_LLAVE")==0){
        element();
        el_prima();    
    }else if(strcmp(t.compLex,"R_CORCHETE")==0){
        //si el token es un corchete cerrado
        sprintf(mensaje,"Se esperaba  se esperaba los siguientes tokens \"[\" o \"{\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }else 
        getToken();    
}

void el_prima(){
    //si el token es una coma
    if(strcmp(t.compLex,"COMA")==0){
        match("COMA");
        pos_tab++; // Aumenta el nivel de tabulación ante un elemento.
        element();
        el_prima();
    }
    else if(strcmp(t.compLex,"R_CORCHETE")!=0){       
        //si el token es un corchete cerrado
        getToken(); 
    }
}

void object(){
    //si el token es una llave abierta
    if(strcmp(t.compLex,"L_LLAVE")==0){
        match("L_LLAVE");
        o_prima(); 
        pos_tab--; // Cuando el object termina se baja una posicion la tabulación.
    }
    else if(strcmp(t.compLex,"R_CORCHETE")==0||strcmp(t.compLex,"R_LLAVE")==0||strcmp(t.compLex,"COMA")==0){
        //si el token es un corchete cerrado, una llave cerrada o una coma, significa error
        sprintf(mensaje,"Se esperaba se esperaba los siguientes tokens \"{\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }
    else getToken();
}

void o_prima(){
    //si el token es una llave cerrada
    if(strcmp(t.compLex,"R_LLAVE")==0){
        match("R_LLAVE");   
    }else if(strcmp(t.compLex,"LITERAL_CADENA")==0){
        //si el token es un string
        attribute_list(); 
        match("R_LLAVE");
    }else if(strcmp(t.compLex,"R_CORCHETE")==0||strcmp(t.compLex,"COMA")==0){
        //si el token es un corchete cerrado o una coma
        sprintf(mensaje,"Se esperaba se esperaba los siguientes tokens \"}\" o \"string\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }else 
        getToken();     
}

void attribute_list(){
    //si el token es un string
    if(strcmp(t.compLex,"LITERAL_CADENA")==0){
        attribute();
        al_prima(); 
    }else if(strcmp(t.compLex,"R_LLAVE")==0){
        //si el token es una llave cerradas
        sprintf(mensaje,"Se esperaba los siguientes tokens \"string\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }else 
        getToken(); 
}

void attribute(){
    if(strcmp(t.compLex,"LITERAL_CADENA")==0){
        // Puntero auxiliar para extraer las commillas.
        char* aux= calloc(1000,sizeof(char));
        aux=t.pe->lexema;
        aux++;
        aux[strlen(aux)-1] = '\0';
        
        sprintf(trad,"<%s>",aux);
		
		// Tabulador devuelve las tabulaciones según el nivel
        strcpy(trad,tabulador(trad)); 
		
		// Se escribe en el archivo;
        fputs(trad,salida); 
    
		//se evalua el nombre del atributo
		attribute_name();
        match("DOS_PUNTOS");
		
		//se evalua el valor del atributo
        attribute_value(); 
		
		// Se remueve todas las tabulaciones.
        destabulador(trad); 

		// Se escribe en el archivo;
        sprintf(trad,"</%s>\n", aux);
        fputs(trad,salida); 
    }
    else if(strcmp(t.compLex,"R_LLAVE")==0||strcmp(t.compLex,"COMA")==0){
        sprintf(mensaje,"Se esperaba los siguientes tokens \"string\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }
    else getToken();    
}

void al_prima(){
    //si el token es una coma
    if(strcmp(t.compLex,"COMA")==0){
        match("COMA");
        attribute();
        al_prima();
    }else if(strcmp(t.compLex,"R_LLAVE")!=0){ 
        //si el token es una llave cerrada
        getToken(); 
    }       
}

void attribute_name(){
    //se espera un string
    if(strcmp(t.compLex,"LITERAL_CADENA")==0){
        match("LITERAL_CADENA");
    }else
        getToken();
}


void attribute_value(){
	//se espera una llave abierta
    if(strcmp(t.compLex,"L_LLAVE")==0||strcmp(t.compLex,"L_CORCHETE")==0){
        fputs("\n",salida);
        pos_tab++; // Aumenta de nivel por elemento en el archivo fuente.
        element();
    }
    else if(strcmp(t.compLex,"LITERAL_CADENA")==0){
        fputs(t.pe->lexema,salida);
        match("LITERAL_CADENA");
    }//se espera un NUM
    else if(strcmp(t.compLex,"LITERAL_NUM")==0){
        fputs(t.pe->lexema,salida);
        match("LITERAL_NUM");
    }//se espera un TRUE
    else if(strcmp(t.compLex,"PR_TRUE")==0){
        fputs(t.pe->lexema,salida);
        match("PR_TRUE");
    }//se espera un FALSE
    else if(strcmp(t.compLex,"PR_FALSE")==0){
        fputs(t.pe->lexema,salida);
        match("PR_FALSE");
    }//se espera un NULL
    else if(strcmp(t.compLex,"PR_NULL")==0){
        fputs(t.pe->lexema,salida);
        match("PR_NULL");
    }//caso contrario
    else if(strcmp(t.compLex,"R_LLAVE")==0||strcmp(t.compLex,"COMA")==0||strcmp(t.compLex,"DOS_PUNTOS")==0){
        sprintf(mensaje,"Se esperaba los siguientes tokens \"{\" o \"[\" o \"string\" o \"number\" o \"true\" o \"false\" o \"null\" no \"%s\"", t.pe->lexema);
        error_mensaje(mensaje);
    }
    else getToken(); 
}

void inicio(void){
    json(); 
     if(strcmp(t.compLex,"EOF")!=0) error_mensaje("ERROR: No se espera EOF.");
}

void parser(){
    while (strcmp(t.compLex,"EOF")!=0){
        getToken();
        inicio();
    }   
}

int main(int argc,char* args[]){
    //se inicializa la tabla de simbolos
    initTabla();

    //se carga la tabla de simbolos
    initTablaSimbolos();
     
    if(argc > 1){

        //se verifica la direccion y nombre del archivo    
    	if ( !( archivo=fopen(args[1],"rt") ) ){
                printf("Archivo no encontrado.\n");
                exit(1);
    	}
		while (strcmp(t.compLex,"EOF")!=0){
            salida = fopen( "salida.txt", "w" );
            parser();
			//si no se encontraron errores en la fuente
            if(bandera_error==0)
                printf("La sintaxis de la fuente es correcta.\n");
            else
                printf("La sintaxis de la fuente no es correcta\n");
			fclose(salida);
		}
		//se cierra el archivo
		fclose(archivo);
	}else{//no se paso el nombre del archivo fuente
    	printf("Debe pasar la direccion del archivo fuente\n");
    	exit(1);
    }
    return 0;
}
