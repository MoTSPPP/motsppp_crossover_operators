#ifndef CROSSOVER_CPP
#define CROSSOVER_CPP

#include "crossover.h"
#include "../main.h"
#include "../utils/solution_utils.h"

#include <cstdlib>

Solution one_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother) {
  Solution baby;
  for (int i = 0; i < (int)population[father].route.size() / 2; i++) {
    baby.route.push_back(population[father].route[i]);
    baby.cities_colected.push_back(population[father].cities_colected[i]);
  }
  for (int j = (int)population[mother].route.size() / 2;
       j < (int)population[mother].route.size(); j++) {
    bool gene_is_repeated = false;
    for (int k = 0; k < (int)population[father].route.size() / 2; k++) {
      if (population[mother].route[j] == population[father].route[k]) {
        gene_is_repeated = true;
      }
    }
    if (gene_is_repeated == false) {
      baby.route.push_back(population[mother].route[j]);
      baby.cities_colected.push_back(population[mother].cities_colected[j]);
    }
  }
  return baby;
}

Solution one_crossover(Instance instance, std::vector<Solution>& population) {
  int father = rand() % population.size();
  int mother = rand() % population.size();
  return one_crossover(instance, population, father, mother);
}

Solution random_point_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2) {
  // ponto de corte sorteado, substitui o antigo pivo fixo em size/2
  int pivot = rand() % (int)parent1.route.size();

  Solution baby;
  for (int i = 0; i < pivot; i++) {
    baby.route.push_back(parent1.route[i]);
    baby.cities_colected.push_back(parent1.cities_colected[i]);
  }
  // se o segundo pai nao tiver posicoes suficientes para o pivo sorteado, ele e ignorado
  if ((int)parent2.route.size() > pivot) {
    for (int j = pivot; j < (int)parent2.route.size(); j++) {
      bool gene_is_repeated = false;
      for (int k = 0; k < pivot; k++) {
        if (parent2.route[j] == parent1.route[k]) {
          gene_is_repeated = true;
        }
      }
      if (gene_is_repeated == false) {
        baby.route.push_back(parent2.route[j]);
        baby.cities_colected.push_back(parent2.cities_colected[j]);
      }
    }
  }
  return baby;
}

std::vector<Solution> crossover(Instance& instance, Population& parents) {
  std::vector<Solution> children;
  while (children.size() < parents.population.size()) {
    Solution baby = one_crossover(instance, parents.population);
    update_objectives(instance, baby);
    children.push_back(baby);
  }
  return children;
}

Solution one_priority_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2){
    //crossover igual ao original (one_crossover), mas com prioridade para o pai que tiver mais passageiros a bordo. 
    //Se ambos tiverem o mesmo número de passageiros, o primeiro é priorizado.
    const Solution& priorized_parent =
        std::count(parent1.passengers_riding.begin(), parent1.passengers_riding.end(), true)
        >=
        std::count(parent2.passengers_riding.begin(), parent2.passengers_riding.end(), true)
            ? parent1: parent2;
    const Solution& non_priorized_parent = (&priorized_parent == &parent1) ? parent2 : parent1;

    Solution baby;
    for (int i = 0; i < (int)priorized_parent.route.size() / 2; i++) {
        baby.route.push_back(priorized_parent.route[i]);
        baby.cities_colected.push_back(priorized_parent.cities_colected[i]);
    }
    for (int j = (int)non_priorized_parent.route.size() / 2;
        j < (int)non_priorized_parent.route.size(); j++) {
        bool gene_is_repeated = false;
        for (int k = 0; k < (int)priorized_parent.route.size() / 2; k++) {
        if (non_priorized_parent.route[j] == priorized_parent.route[k]) {
            gene_is_repeated = true;
        }
        }
        if (gene_is_repeated == false) {
        baby.route.push_back(non_priorized_parent.route[j]);
        baby.cities_colected.push_back(non_priorized_parent.cities_colected[j]);
        }
    }
    return baby;
}

Solution random_point_priority_crossover(const Instance& instance, const Solution& parent1, const Solution& parent2){
    //crossover igual ao original (one_crossover), mas com prioridade para o pai que tiver mais passageiros a bordo. 
    //Se ambos tiverem o mesmo número de passageiros, o primeiro é priorizado.
    const Solution& priorized_parent =
        std::count(parent1.passengers_riding.begin(), parent1.passengers_riding.end(), true)
        >=
        std::count(parent2.passengers_riding.begin(), parent2.passengers_riding.end(), true)
            ? parent1: parent2;
    const Solution& non_priorized_parent = (&priorized_parent == &parent1) ? parent2 : parent1;

    // ponto de corte sorteado, substitui o antigo pivo fixo em size/2
    int pivot = rand() % (int)priorized_parent.route.size();

    Solution baby;
    for (int i = 0; i < pivot; i++) {
        baby.route.push_back(priorized_parent.route[i]);
        baby.cities_colected.push_back(priorized_parent.cities_colected[i]);
    }
    // se o pai nao-priorizado nao tiver posicoes suficientes para o pivo sorteado, ele e ignorado
    if ((int)non_priorized_parent.route.size() > pivot) {
        for (int j = pivot; j < (int)non_priorized_parent.route.size(); j++) {
            bool gene_is_repeated = false;
            for (int k = 0; k < pivot; k++) {
            if (non_priorized_parent.route[j] == priorized_parent.route[k]) {
                gene_is_repeated = true;
            }
            }
            if (gene_is_repeated == false) {
            baby.route.push_back(non_priorized_parent.route[j]);
            baby.cities_colected.push_back(non_priorized_parent.cities_colected[j]);
            }
        }
    }
    return baby;
}

// checa se a cidade e origem ou destino de algum passageiro que estava
// efetivamente embarcado na solucao doadora (ou seja, se inserir essa
// cidade tem chance de reativar aquela carona no filho)
static bool is_passenger_relevant_city(const Instance& instance, const Solution& donor, int city) {
    if (donor.passengers_riding.empty()) return false;
    for (int p = 0; p < instance.number_of_passengers; p++) {
        if (donor.passengers_riding[p] &&
            (instance.passengers[p].origin == city || instance.passengers[p].destiny == city)) {
            return true;
        }
    }
    return false;
}

// crossover guiado por passageiros: comeca com a rota inteira do pai (espinha),
// preservando toda a estrutura de bonus/caronas que ja funcionava nele, e so
// importa da mae (doadora) as cidades que estavam servindo de embarque/desembarque
// de algum passageiro efetivamente embarcado la, inserindo-as na posicao
// relativa em que apareciam na doadora e com o bonus dessas cidades novas
// sempre marcado como coletado.
Solution passenger_guided_insertion_crossover(const Instance& instance, const Solution& spine, const Solution& donor) {
    Solution baby;
    baby.route = spine.route;
    baby.cities_colected = spine.cities_colected;

    vector<bool> visited(instance.number_of_cities, false);
    for (int city : baby.route) {
        visited[city] = true;
    }

    int anchor_pos = 0; // posicao em baby.route apos a qual a proxima insercao acontece
    for (int j = 1; j < (int)donor.route.size(); j++) {
        int city = donor.route[j];
        if (visited[city]) {
            // ja esta no filho: so atualiza a ancora pra manter a ordem relativa da doadora
            for (int k = 0; k < (int)baby.route.size(); k++) {
                if (baby.route[k] == city) {
                    anchor_pos = k;
                    break;
                }
            }
            continue;
        }
        if (is_passenger_relevant_city(instance, donor, city)) {
            baby.route.insert(baby.route.begin() + anchor_pos + 1, city);
            // cidade nova entra com o bonus sempre coletado, ja que ela so foi
            // inserida por causa de uma carona - aproveitar o bonus e de graca
            baby.cities_colected.insert(baby.cities_colected.begin() + anchor_pos + 1, true);
            visited[city] = true;
            anchor_pos++;
        }
    }
    return baby;
}

Solution passenger_guided_insertion_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother) {
    return passenger_guided_insertion_crossover(instance, population[father], population[mother]);
}

Solution passenger_guided_insertion_crossover(Instance instance, std::vector<Solution>& population) {
    int father = rand() % population.size();
    int mother = rand() % population.size();
    return passenger_guided_insertion_crossover(instance, population[father], population[mother]);
}

// define a estrutura de um bloco pra facilitar a vida no crossover
struct Block {
    std::vector<int> route_segment;
    std::vector<bool> bonus_segment;
    int load = 0;
};

// extrai os blocos de um pai baseado nos embarques e desembarques
std::vector<Block> extract_blocks(const Instance& instance, const Solution& sol) {
    std::vector<Block> blocks;
    if (sol.route.size() <= 1) return blocks;

    Block current_block;
    int current_passengers = 0;

    // a gente ignora o 0 porque ele nao entra na logica de blocos
    for (size_t i = 1; i < sol.route.size(); ++i) {
        int city = sol.route[i];
        bool event_happened = false;
        int pass_diff = 0;

        // verifica se algum passageiro que esta na solucao sobe ou desce aqui
        for (int p = 0; p < instance.number_of_passengers; ++p) {
            if (sol.passengers_riding[p]) {
                if (instance.passengers[p].origin == city) {
                    event_happened = true;
                    pass_diff++;
                }
                if (instance.passengers[p].destiny == city) {
                    event_happened = true;
                    pass_diff--;
                }
            }
        }

        // se alguem subiu ou desceu, fecha o bloco atual (se nao tiver vazio) e comeca um novo
        if (event_happened && !current_block.route_segment.empty()) {
            current_block.load = current_passengers * current_block.route_segment.size();
            blocks.push_back(current_block);
            current_block.route_segment.clear();
            current_block.bonus_segment.clear();
        }

        // atualiza a lotacao do carro pro bloco que vai ser formado
        current_passengers += pass_diff;

        current_block.route_segment.push_back(city);
        current_block.bonus_segment.push_back(sol.cities_colected[i]);
    }

    // nao esquece de salvar o ultimo trecho que sobrou
    if (!current_block.route_segment.empty()) {
        current_block.load = current_passengers * current_block.route_segment.size();
        blocks.push_back(current_block);
    }

    return blocks;
}

// o crossover de fato
Solution one_block_crossover(const Instance& instance, const Solution& spine, const Solution& donor) {
    Solution baby; 
    
    // a rota do filho sempre comeca no 0, independente de qualquer bloco
    baby.route.push_back(0);
    baby.cities_colected.push_back(false); // origens geralmente nao tem bonus

    std::vector<Block> spine_blocks = extract_blocks(instance, spine);
    std::vector<Block> donor_blocks = extract_blocks(instance, donor);

    // ordena o doador pelo maior load
    std::sort(donor_blocks.begin(), donor_blocks.end(), [](const Block& a, const Block& b) {
        return a.load > b.load;
    });

    // vetor pra rastrear quem ja entrou e nao repetir cidade
    std::vector<bool> visited(instance.number_of_cities, false);
    visited[0] = true;

    // o numero de comparacoes e limitado pelo pai que tiver menos blocos
    int num_comparisons = std::min(spine_blocks.size(), donor_blocks.size());

    for (int i = 0; i < num_comparisons; ++i) {
        const Block& sb = spine_blocks[i];
        const Block& db = donor_blocks[i];

        const Block* winner = nullptr;

        // se o doador for estritamente maior, ele ganha. empate vai pra espinha.
        if (db.load > sb.load) {
            winner = &db;
        } else {
            winner = &sb;
        }

        // confere se o bloco vencedor tenta colocar alguma cidade que ja ta na rota
        bool valid = true;
        for (int city : winner->route_segment) {
            if (visited[city]) {
                valid = false;
                break;
            }
        }

        // se o bloco ta limpo, empurra ele pro filho. se for invalido, e descartado e ninguem entra.
        if (valid) {
            for (size_t j = 0; j < winner->route_segment.size(); ++j) {
                int city = winner->route_segment[j];
                baby.route.push_back(city);
                baby.cities_colected.push_back(winner->bonus_segment[j]);
                visited[city] = true;
            }
        }
    }

    return baby;
}

Solution one_block_crossover(const Instance& instance, std::vector<Solution>& population, int father, int mother){
    return one_block_crossover(instance, population[father], population[mother]);
}

// sobrecarga que escolhe os pais e chama o novo crossover
Solution one_block_crossover(Instance instance, std::vector<Solution>& population) {
    int father = rand() % population.size();
    int mother = rand() % population.size();
    // nesse caso, o pai sorteado primeiro sera a espinha, e a mae o doador
    return one_block_crossover(instance, population[father], population[mother]);
}

std::vector<Solution> block_crossover(Instance& instance, Population& parents) {
  std::vector<Solution> children;
  while (children.size() < parents.population.size()) {
    Solution baby = one_block_crossover(instance, parents.population);
    update_objectives(instance, baby);
    children.push_back(baby);
  }
  return children;
}

#endif
