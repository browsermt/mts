
/* Operates on a notion of sentences and converts them into batches. 
 * Some ProducerConsumer logic here. 
 * 
 * Producer: 
 *    Sequentialize all incoming requests (group of sentences) with a Queue. 
 *    These sentences can be from several "requests".
 *    Will need mechanism to have priority/staleness to schedule.
 *    Requests (ordering) is lost when sorting happens to optimize batch size.
 *    Producer units are batches. Will need accounting to keep track and put back.
 *   
 * 
 * Consumer: The worker mechanism of
 *    BatchTranslators are the pool of consumers.
 *    
 */

class Batcher {

};