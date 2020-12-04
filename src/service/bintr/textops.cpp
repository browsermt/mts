#include "textops.h"

namespace marian
{
  namespace bergamot
  {
    SentenceSplitter::SentenceSplitter(marian::Ptr<marian::Options> options) : options_(options)
    {
      auto ssplit_prefix_file =
          options_->get<std::string>("ssplit-prefix-file", "");

      if (ssplit_prefix_file.size())
      {
        ssplit_prefix_file = marian::cli::interpolateEnvVars(ssplit_prefix_file);
        LOG(info, "Loading protected prefixes for sentence splitting from {}",
            ssplit_prefix_file);
        ssplit_.load(ssplit_prefix_file);
      }
      else
      {
        LOG(warn,
            "Missing list of protected prefixes for sentence splitting. "
            "Set with --ssplit-prefix-file.");
      }
    }

    ug::ssplit::SentenceStream
    SentenceSplitter::createSentenceStream(
        std::string const &input,
        ug::ssplit::SentenceStream::splitmode const &mode)
    {
      return std::move(ug::ssplit::SentenceStream(input, this->ssplit_, mode));
    }

    ug::ssplit::SentenceStream::splitmode SentenceSplitter::string2splitmode(
        const std::string &m, bool throwOnError /*=false*/)
    {
      typedef ug::ssplit::SentenceStream::splitmode splitmode;
      // @TODO: throw Exception on error
      if (m == "sentence" || m == "Sentence")
        return splitmode::one_sentence_per_line;
      if (m == "paragraph" || m == "Paragraph")
        return splitmode::one_paragraph_per_line;
      if (m != "wrapped_text" && m != "WrappedText" && m != "wrappedText")
      {
        LOG(warn, "Ignoring unknown text input format specification: {}.", m);
      }
      return splitmode::wrapped_text;
    }
  } // namespace bergamot
} // namespace marian