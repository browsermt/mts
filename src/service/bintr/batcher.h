
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

public:
  unsigned int max_input_tokens_;
  unsigned int current_input_tokens_;


  Batcher(Ptr<Options> options): current_input_tokens_(0) {
    /* Reads options for batch configuration */
    max_input_tokens_ = options->get<int>("max-input-tokens");
  }

  void add_segment(Words &segment){

  }

  std::vector<data::SentenceTuple> cleave_batch(){
    /* Construct one-to-one mapping here */
  }
};