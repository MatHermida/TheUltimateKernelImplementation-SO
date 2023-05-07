#include "../include/planificacion_largo.h"

void planificar_largo() {
	while(1) {
		sem_wait(&sem_cant_new); //para que no haga nada si no hay nadie en new
		sem_wait(&sem_multiprogramacion);

		//Lo dejo comentado por si quiero ver el grado de multiprogramacion:

		//int grado_actual_multiprogramacion;
		//sem_getvalue(&sem_multiprogramacion, &grado_actual_multiprogramacion);
		//log_warning(logger, "sem_multiprogramacion: %d", grado_actual_multiprogramacion);

		if(queue_size(new_queue) > 0) { //Este if no hace falta
			t_pcb* pcb = queue_pop_con_mutex(new_queue, &mutex_new_queue);
			pcb->tiempo_llegada_ready = time(NULL);

			list_push_con_mutex(ready_list, pcb, &mutex_ready_list);
			log_info(logger, "PID: %d - Estado Anterior: NEW - Estado Actual: READY", pcb->pid); //log obligatorio
			log_pids(); //log obligatorio

			sem_post(&sem_cant_ready);
		}
		else {
			log_error(logger, "Error en la lista NEW");
		}
	}
}
