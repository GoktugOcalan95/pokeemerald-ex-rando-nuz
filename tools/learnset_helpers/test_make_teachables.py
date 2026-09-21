import re
import unittest

from make_teachables import prepare_output


class TeachingCompatibilityTests(unittest.TestCase):
    def generate(self, tms, tutors):
        source = {
            'BAXCALIBUR': ['MOVE_THUNDER_FANG', 'MOVE_ICICLE_SPEAR'],
            'MAGIKARP': ['MOVE_SPLASH'],
            'TERAPAGOS': ['MOVE_TERA_BLAST'],
        }
        special = {
            'universalMoves': ['MOVE_HIDDEN_POWER', 'MOVE_TERA_BLAST'],
            'signatureTeachables': ['MOVE_DRAGON_ASCENT'],
        }
        species = [
            {'name': 'Baxcalibur', 'teaching_type': 'DEFAULT_LEARNING'},
            {'name': 'Magikarp', 'teaching_type': 'TM_ILLITERATE'},
            {'name': 'Mew', 'teaching_type': 'ALL_TEACHABLES'},
            {'name': 'Terapagos', 'teaching_type': 'DEFAULT_LEARNING'},
        ]
        output = prepare_output(source, tms, tutors, special, species, '')
        return {name: re.findall(r'MOVE_\w+', body)[:-1]
                for name, body in re.findall(r'static const u16 s(\w+)\[\] = \{(.*?)\};', output, re.S)}

    def test_source_compatibility_is_independent_of_fixed_teaching_sources(self):
        empty = self.generate([], [])
        offered = self.generate(['MOVE_THUNDER_FANG'], ['MOVE_ICICLE_SPEAR'])
        self.assertEqual(empty['BaxcaliburFixedTeachingMoves'], [])
        self.assertEqual(offered['BaxcaliburFixedTeachingMoves'], ['MOVE_THUNDER_FANG', 'MOVE_ICICLE_SPEAR'])
        self.assertEqual(empty['BaxcaliburMoveCompatibility'], offered['BaxcaliburMoveCompatibility'])
        self.assertIn('MOVE_THUNDER_FANG', empty['BaxcaliburMoveCompatibility'])
        self.assertIn('MOVE_ICICLE_SPEAR', empty['BaxcaliburMoveCompatibility'])

    def test_species_exceptions_apply_to_full_compatibility(self):
        tables = self.generate([], [])
        self.assertEqual(tables['MagikarpMoveCompatibility'], ['MOVE_SPLASH'])
        self.assertNotIn('MOVE_TERA_BLAST', tables['TerapagosMoveCompatibility'])
        self.assertIn('MOVE_THUNDER_FANG', tables['MewMoveCompatibility'])
        self.assertNotIn('MOVE_DRAGON_ASCENT', tables['MewMoveCompatibility'])
        self.assertNotIn('MOVE_STRUGGLE', tables['MewMoveCompatibility'])
        self.assertNotIn('MOVE_NONE', tables['MewMoveCompatibility'])

    def test_duplicate_fixed_sources_do_not_duplicate_moves(self):
        tables = self.generate(['MOVE_THUNDER_FANG'], ['MOVE_THUNDER_FANG'])
        self.assertEqual(tables['BaxcaliburFixedTeachingMoves'], ['MOVE_THUNDER_FANG'])
        for moves in tables.values():
            self.assertEqual(len(moves), len(set(moves)))


if __name__ == '__main__':
    unittest.main()
