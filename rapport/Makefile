PDF = $(patsubst %.markdown,%.pdf,$(wildcard *.markdown))
PANDOC = pandoc
PANDOC_FLAGS = -s

all: $(PDF)

%.pdf: %.markdown
	$(PANDOC) $(PANDOC_FLAGS) $< -o $@

clean:
	rm -rf $(PDF)
